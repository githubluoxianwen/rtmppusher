#include "libaac_api.h"
#include "sys/sys_mem.h"
#include "stdlib.h"
#include "sys/sys_pthread.h"

#define AAC_BUFFER_SIZE	500
#define G711A_BUFFER_SIZE 800 * 5 + 1024

static short decode(unsigned char alaw)
{
	alaw ^= 0xD5;
	int sign = alaw & 0x80;
	int exponent = (alaw & 0x70) >> 4;
	int data = alaw & 0x0f;
	data <<= 4;
	data += 8;
	if (exponent != 0)
		data += 0x100;
	if (exponent > 1)
		data <<= (exponent - 1);

	return (short)(sign == 0 ? data : -data);
}

static int g711_decode(char* pRawData, const unsigned char* pBuffer, int nBufferSize)
{
	short *out_data = (short*)pRawData;
	int i;
	for (i = 0; i<nBufferSize; i++)
	{
		out_data[i] = decode(pBuffer[i]);
	}

	return nBufferSize * 2;
}

static int send_AAC_spec(AAC* aac_handle, RTMP *rtmp_handle)
{
	int ret = 0;
	int rate_idx = 0;
	char seq_head[4] = { 0 };

	for (rate_idx = 0; rate_idx < HZ_END; rate_idx++)
	{
		if (aac_handle->sample_rate == sample_rate_table[rate_idx])
			break;
	}

	if (rate_idx == HZ_END)
	{
		RTMP_LOGE("not find such sample rate! [=%d] user default 8000\n", aac_handle->sample_rate);
		rate_idx = 11;
	}

	if (ret == 0)
	{
		//flv audio tag
		seq_head[0] = 0xAF;//����aac�����ŵĲ���������44.1kh,��������������,ѹ���������Ƶ����16bit
		 //aac data type : sequence data	
		seq_head[1] = 0x00;
		//aal_lc | 8khz | ������ | ����
		//bit:  5 | 4 | 4 | 3
		//seq_head[2] = 0x15;
		//seq_head[3] = 0x88;
		seq_head[2] = (AAC_LC << 3) | (rate_idx >> 1);
		seq_head[3] = (rate_idx << 7) | (aac_handle->channels << 3);
		//rtmp audio start packet
		ret = rtmp_api_sendPacket(rtmp_handle, RTMP_PACKET_TYPE_AUDIO, seq_head, sizeof(seq_head), 0, false);
	}
	return ret;
}

//����NVR��Ƶ�ַ���ʱ���д���С�����ݲ����ȣ�������������ͻȻ��һ�����һС��û��������������ǲ�Ӱ�첥��
int aac_api_sendAACPacket(AAC *aac_handle, RTMP *rtmp_handle, char *_data, unsigned int _size, unsigned int encoder_type, unsigned int is_first_data, unsigned int time_delta, unsigned int *_nTimeStamp)
{
	if (_data == NULL && _size < 11) 
		return -1;

	int ret = 0;
	int sample_count = 0;
	int buff_size = AAC_BUFFER_SIZE;
	unsigned char *aac_buff = NULL;
	int aac_data_len = 0;

	aac_buff = (unsigned char *)sys_malloc(buff_size);
	if (NULL == aac_buff)
	{
		RTMP_LOGE("error: malloc aac buffer failed!\n");
		return -1;
	}
	memset(aac_buff, 0, buff_size);

	if (0 == ret && is_first_data)
	{
		//������Ƶ��ʼ����rtmp audio start | flv audio  start | aac audio start��
		ret = send_AAC_spec(aac_handle,rtmp_handle);
	}

	if (0 == ret)
	{
		//��Ƶ���ݽ�g711a����
		if (_size + aac_handle->g711a_len > G711A_BUFFER_SIZE)
		{
			//��Ƶ̫�󣬶��������
			memcpy(aac_handle->g711a_buffer + aac_handle->g711a_len, _data, G711A_BUFFER_SIZE - aac_handle->g711a_len);
			aac_handle->g711a_len = G711A_BUFFER_SIZE;
		}
		else
		{
			memcpy(aac_handle->g711a_buffer + aac_handle->g711a_len, _data, _size);
			aac_handle->g711a_len += _size;
		}

		if (aac_handle->g711a_len < aac_handle->input_samples)
		{
			//g711a��Ƶ������һ֡aac,
			sys_free(aac_buff);
			return 0;
		}
	}

	if (0 == ret)
	{
		//�ָ�g711as����Ϊ������Ҫ����С��λ�����б��벢����
		int i = 0; 
		int one_frame_aac_size = 0;
		int out_pcm_len = 0;
		unsigned int time_stamp = *_nTimeStamp;
		unsigned char *g711a_buffer = aac_handle->g711a_buffer;
		sample_count = (aac_handle->g711a_len / aac_handle->input_samples);
		for (i = 0; i < sample_count; i++)
		{
			if (0x90 == encoder_type)//RJ_ENC_FMT_G711A
				out_pcm_len = g711_decode((char *)aac_handle->PCM_buffer, g711a_buffer, aac_handle->input_samples);
			else
			{
				RTMP_LOGE("audio encode type not G711A! [type is %x]\n",encoder_type);
				ret = -1;
				break;
			}

			if (out_pcm_len > 0)
			{
				int loop = 0;
				unsigned int input_samples = out_pcm_len / (aac_handle->PCM_bit_size / 8);
				g711a_buffer += aac_handle->input_samples;
				//for (loop = 0; loop < 4 && one_frame_aac_size == 0; loop++)//��ʼ����������Ҫ��֡pcm���ܱ��һ֡aac����
				one_frame_aac_size = faacEncEncode(aac_handle->encoder_handle, (int*)aac_handle->PCM_buffer, input_samples, aac_buff+2, buff_size-2);
				
				if (one_frame_aac_size > 0)
				{
					//���һ֡aac��ֱ�ӷ���
					//flv audio tag
					aac_buff[0] = 0xAF;//����aac�����ŵĲ���������44kh					
					aac_buff[1] = 0x01; //aac data type:raw data
					time_stamp += time_delta / sample_count;					
					ret = rtmp_api_sendPacket(rtmp_handle, RTMP_PACKET_TYPE_AUDIO, (const char *)aac_buff, one_frame_aac_size + 2, time_stamp, false);

					memset(aac_buff,0, AAC_BUFFER_SIZE);
					one_frame_aac_size = 0;
					out_pcm_len = 0;
				}
			}
		}
		*_nTimeStamp = time_stamp;
	}	

	//��֡g711a�ָ��������ݷ��뻺�壬�ϲ�����֡����ȥ����
	if (0 == ret)
	{
		int left_data = aac_handle->g711a_len%aac_handle->input_samples;
		if (left_data > 0)
		{
			memcpy(aac_handle->g711a_buffer, aac_handle->g711a_buffer + sample_count * aac_handle->input_samples, left_data);
			aac_handle->g711a_len = left_data;
			memset(aac_handle->g711a_buffer + left_data, 0, G711A_BUFFER_SIZE - left_data);
		}
		else
			aac_handle->g711a_len = 0;
	}

	if (aac_buff)
		free(aac_buff);
	aac_buff = NULL;

	return ret;
}

AAC *aac_api_create(unsigned int sample, unsigned int bit_wide, unsigned int track)
{
	if (track >= TRACK_END)
		return NULL;

	AAC *aac_handle = (AAC *)sys_malloc(sizeof(AAC));
	if (NULL == aac_handle)
	{
		RTMP_LOGE("malloc aac failed!\n");
		return NULL;
	}

	aac_handle->sample_rate = sample;
	aac_handle->channels = track;
	aac_handle->PCM_bit_size = bit_wide;
	aac_handle->input_samples = 0;
	aac_handle->max_output_bytes = 0;
	aac_handle->PCM_buffer_size = 0;
	aac_handle->PCM_buffer = NULL;
	aac_handle->g711a_buffer = NULL;
	aac_handle->g711a_len = 0;
	aac_handle->encoder_handle = NULL;
	aac_handle->last_time_sample = 0;

	// (1) Open FAAC engine
	aac_handle->encoder_handle = faacEncOpen(aac_handle->sample_rate, aac_handle->channels, &aac_handle->input_samples, &aac_handle->max_output_bytes);
	if (aac_handle->encoder_handle == NULL)
	{
		RTMP_LOGE("[error] Failed to call faacEncOpen()\n");
		sys_free(aac_handle);
		aac_handle = NULL;
		return NULL;
	}

	aac_handle->PCM_buffer_size = (aac_handle->input_samples * (aac_handle->PCM_bit_size / 8));
	if (aac_handle->PCM_buffer_size <= 0)
	{
		RTMP_LOGE("[error] aac_handle->PCM_buffer_size ERROR\n");
		sys_free(aac_handle);
		aac_handle = NULL;
		return NULL;
	}
	// ���ڱ���һ֡AAC����Ҫ4096���ֽڵ�PCM������������Ҫһ����ʱ�����������ݴ����ǰ������  
	aac_handle->PCM_buffer = (unsigned char *)sys_malloc(aac_handle->PCM_buffer_size);
	if (NULL == aac_handle->PCM_buffer)
	{
		RTMP_LOGE("[error] malloc pcm buffer failed!\n");
		sys_free(aac_handle);
		aac_handle = NULL;
		return NULL;
	}

	aac_handle->g711a_buffer = (unsigned char *)sys_malloc(G711A_BUFFER_SIZE);
	if (NULL == aac_handle->g711a_buffer)
	{
		RTMP_LOGE("[error] malloc g711a buffer failed!\n");
		sys_free(aac_handle->PCM_buffer);
		aac_handle->PCM_buffer = NULL;

		sys_free(aac_handle);
		aac_handle = NULL;
		return NULL;
	}

	memset(aac_handle->PCM_buffer, 0, aac_handle->PCM_buffer_size);
	memset(aac_handle->g711a_buffer, 0, G711A_BUFFER_SIZE);

	// (2) Get current encoding configuration
	aac_handle->configuration = faacEncGetCurrentConfiguration(aac_handle->encoder_handle);
	aac_handle->configuration->inputFormat = FAAC_INPUT_16BIT;
	aac_handle->configuration->outputFormat = 0;//0-Raw ; 1-ADTS �Ƿ����ADTSͷ��Audio Data Transport Stream������aac�Ľ�����Ϣһ���߸��ֽڣ���
	aac_handle->configuration->useTns = 0;//ʱ����������
	aac_handle->configuration->useLfe = 0;//��ƵЧ��
	aac_handle->configuration->aacObjectType = LOW; //aac_lc
	aac_handle->configuration->shortctl = SHORTCTL_NORMAL;
	aac_handle->configuration->quantqual = 100; // ��������  
	//aac_handle->configuration->bandWidth = 0; //Ƶ��
	//aac_handle->configuration->bitRate = 0;
	aac_handle->configuration->mpegVersion = MPEG4;

	// (3) Set encoding configuration
	int nRet = faacEncSetConfiguration(aac_handle->encoder_handle, aac_handle->configuration);
	if (0 == nRet)
	{
		RTMP_LOGE("[error] faac set config failed!\n");
		sys_free(aac_handle->g711a_buffer);
		aac_handle->g711a_buffer = NULL;

		sys_free(aac_handle->PCM_buffer);
		aac_handle->PCM_buffer = NULL;

		sys_free(aac_handle);
		aac_handle = NULL;
		return NULL;
	}

	return aac_handle;
}

int aac_api_destroy(AAC *aac_handle)
{
	if (aac_handle == NULL)
		return -1;

	// Close FAAC engine
	if (aac_handle->encoder_handle != NULL)
	{
		faacEncClose(aac_handle->encoder_handle);
		aac_handle->encoder_handle = NULL;
	}

	if (NULL != aac_handle->g711a_buffer)
	{
		sys_free(aac_handle->g711a_buffer);
		aac_handle->g711a_buffer = NULL;
	}

	if (NULL != aac_handle->PCM_buffer)
	{
		sys_free(aac_handle->PCM_buffer);
		aac_handle->PCM_buffer = NULL;
	}

	sys_free(aac_handle);
	aac_handle = NULL;

	return 0;
}


