#include "librtmp_api.h"
#ifdef WIN32
#include "winsock.h"
#endif


#define RTMP_PACKET_TYPE_CHUNK_SIZE 0x01
#define H264_NAL_SPS  0x67
#define H264_NAL_PPS  0x68
#define H264_NAL_I  0x65
#define H264_NAL_P  0x61
#define H264_NAL_SEI  0x06
#define IS_H264_TYPE(H264_NAL, NAL_TYPE) ((H264_NAL & 0x1F) == (NAL_TYPE & 0x1F))


static int find_start_code(const char *buff, int  data_len, int *start_code_count)
{
	assert(NULL != buff);
	assert(data_len >= 0);

	unsigned int pos = 3;

	while (pos < data_len)
	{
		if (buff[pos - 3] == 0x00 &&
			buff[pos - 2] == 0x00 &&
			buff[pos - 1] == 0x00 &&
			buff[pos] == 0x01)
		{
			*start_code_count = 4;
			return pos - 3;
		}
		else if (buff[pos - 3] == 0x00 &&
			buff[pos - 2] == 0x00 &&
			buff[pos - 1] == 0x01)
		{
			*start_code_count = 3;
			return pos - 3;
		}

		pos++;
	}
	return -1;
}

static void parse_frame_type(const char* data, int data_len, rtmp_h264_info* h264)
{
	assert(NULL != data);
	assert(data_len > 0);
	assert(NULL != h264);

	const int size_num = 4;
	char nal_type = *data;

	if (IS_H264_TYPE(H264_NAL_SPS, nal_type))
	{
		//SPS
		if ((RTMP_SPS_LEN - h264->sps_len - size_num) >= data_len)
		{
			h264->sps_num++;
			char sz_len[size_num] = { 0 };
			sz_len[0] = ((data_len >> 24) & 0xFF);
			sz_len[1] = ((data_len >> 16) & 0xFF);
			sz_len[2] = ((data_len >> 8) & 0xFF);
			sz_len[3] = (data_len & 0xFF);
			memcpy(h264->sps + h264->sps_len, sz_len, size_num);
			h264->sps_len += size_num;
			memcpy(h264->sps + h264->sps_len, data, data_len);
			h264->sps_len += data_len - size_num;
		}
	}
	else if (IS_H264_TYPE(H264_NAL_PPS, nal_type))
	{
		//PPS
		if ((RTMP_PPS_LEN - h264->pps_len - size_num) >= data_len)
		{
			h264->pps_num++;
			char sz_len[size_num] = { 0 };
			sz_len[0] = ((data_len >> 24) & 0xFF);
			sz_len[1] = ((data_len >> 16) & 0xFF);
			sz_len[2] = ((data_len >> 8) & 0xFF);
			sz_len[3] = (data_len & 0xFF);
			memcpy(h264->pps + h264->pps_len, sz_len, size_num);
			h264->pps_len += size_num;
			memcpy(h264->pps + h264->pps_len, data, data_len);
			h264->pps_len += data_len - size_num;
		}
	}
	else if (IS_H264_TYPE(H264_NAL_SEI, nal_type))
	{
//		if ((RTMP_SEI_LEN - h264->sei_len - size_num) >= data_len)
//		{
//			h264->sei_num++;
//			char sz_len[size_num] = { 0 };
//			sz_len[0] = ((data_len >> 24) & 0xFF);
//			sz_len[1] = ((data_len >> 16) & 0xFF);
//			sz_len[2] = ((data_len >> 8) & 0xFF);
//			sz_len[3] = (data_len & 0xFF);
//			memcpy(h264->sei + h264->sei_len, sz_len, size_num);
//			h264->sei_len += size_num;
//			memcpy(h264->sei + h264->sei_len, data, data_len);
//			h264->sei_len += data_len - size_num;
//		}
	}
	else if (IS_H264_TYPE(H264_NAL_I, nal_type))
	{
		//关键帧
		h264->is_key_frame = 1;
		h264->data = (char *)data;
		h264->data_len = data_len;
	}
	else if (IS_H264_TYPE(H264_NAL_P, nal_type))
	{
		//非关键帧
		h264->data = (char *)data;
		h264->data_len = data_len;
	}
	else
	{	
		//RTMP_LOGI("Unknow nalu type:%02x len:%d",nal_type, data_len);
	}

	return;
}

static int read_H264_codec_info(const char* data, int data_len, rtmp_h264_info* p_parsed_buf)
{
	char nal_type = 0;
	int cur_pos = 0;
	int start_code_pos = 0;       // start code 的位置
	const char* frame_buf = data;
	int frame_len = data_len;

	while (cur_pos < data_len)
	{
		int start_code_count = 0;
		//找第一个start code
		start_code_pos = find_start_code(frame_buf, frame_len, &start_code_count);
		if (start_code_pos < 0)
		{
			// 没有找到start code
			return -1;
		}

		cur_pos += start_code_pos + start_code_count;
		frame_len = data_len - cur_pos;
		frame_buf = data + cur_pos;

		//如果解析到I或者P帧的起始码，后面的数据默认没有其他单元了，
		//如果I帧数据很大，继续去遍历会很耗CPU
		nal_type = *frame_buf;
		if (IS_H264_TYPE(H264_NAL_I, nal_type))
		{
			parse_frame_type(frame_buf, frame_len, p_parsed_buf);
			break;
		}
		else if (IS_H264_TYPE(H264_NAL_P, nal_type))
		{
			parse_frame_type(frame_buf, frame_len, p_parsed_buf);
			break;
		}

		start_code_pos = find_start_code(frame_buf, frame_len, &start_code_count);
		if (start_code_pos < 0)
		{
			//只有一个start code
			parse_frame_type(frame_buf, frame_len, p_parsed_buf);
			break;
		}
		else
		{
			parse_frame_type(frame_buf, start_code_pos, p_parsed_buf);
			cur_pos += start_code_pos;
			frame_len = data_len - cur_pos;
			frame_buf = data + cur_pos;

			if (p_parsed_buf->data && p_parsed_buf->data_len)
			{
				break;
			}
		}
	}

	return 0;
}


int rtmp_api_sendPacket(RTMP *handle, unsigned int _nPacketType, const char *_data, unsigned int _size, unsigned int _nTimestamp, bool p_bQueue)
{
	if (handle == NULL) {
		return -1;
	}

	RTMPPacket packet;
	RTMPPacket_Reset(&packet);
	RTMPPacket_Alloc(&packet, _size);

	if (0 == _nTimestamp)
		packet.m_headerType = RTMP_PACKET_SIZE_LARGE;//时间戳归零的时候需要发0类型的消息
	else
		packet.m_headerType = RTMP_PACKET_SIZE_MEDIUM;//同一个流的消息发送1类型的消息

	packet.m_packetType = _nPacketType;
	packet.m_nChannel = 0x04; //消息ID  0，1，2，3保留，用户只能使用4以上的
	packet.m_hasAbsTimestamp = 0;
	packet.m_nTimeStamp = _nTimestamp;
	packet.m_nInfoField2 = handle->m_stream_id;
	packet.m_nBodySize = _size;
	
	memcpy(packet.m_body, _data, _size);

	//printf("type=%x time_stamp=%d\n", _nPacketType, _nTimestamp);

	int nRet = RTMP_SendPacket(handle, &packet, p_bQueue);
	RTMPPacket_Free(&packet);
	if (nRet == 0) {
		return -1;
	}
	return 0;
}

int rtmp_change_chunk_size(RTMP *r, int size)
{
	RTMPPacket packet;
	char pbuf[RTMP_MAX_HEADER_SIZE + 4];

	packet.m_nBytesRead = 0;
	packet.m_body = pbuf + RTMP_MAX_HEADER_SIZE;

	packet.m_packetType = RTMP_PACKET_TYPE_CHUNK_SIZE;
	packet.m_nChannel = 0x04;
	packet.m_headerType = RTMP_PACKET_SIZE_LARGE;
	packet.m_nTimeStamp = 0;
	packet.m_nInfoField2 = 0;
	packet.m_hasAbsTimestamp = 0;
	packet.m_nBodySize = 4;
	r->m_outChunkSize = size;

	r->m_outChunkSize = htonl(r->m_outChunkSize);

	memcpy(packet.m_body, &r->m_outChunkSize, 4);

	r->m_outChunkSize = ntohl(r->m_outChunkSize);

	return RTMP_SendPacket(r, &packet, TRUE);
}

static int send_AVC_sequence_packet(RTMP *handle, rtmp_h264_info *h264)
{
	if (NULL == handle)
	{
		RTMP_LOGE("rtmp handle is null!\n");
		return -1;
	}

	int i = 0;
	int ret = 0;
	char body[1024] = { 0 };

	//构造AVC视频同步包
	if (h264->is_key_frame)
	{
		//AVC Sequence Header
		body[i++] = 0x17;
		body[i++] = 0x00;
		body[i++] = 0x00;
		body[i++] = 0x00;
		body[i++] = 0x00;

		//AVCDecoderConfigurationRecord
		body[i++] = 0x01;
		body[i++] = h264->sps[1 + 4];
		body[i++] = h264->sps[2 + 4];
		body[i++] = h264->sps[3 + 4];
		body[i++] = 0xff;

		body[i++] = 0xE1;
		body[i++] = (h264->sps_len >> 8) & 0xFF;
		body[i++] = h264->sps_len & 0xFF;
		memcpy(&body[i], h264->sps + 4, h264->sps_len);
		i += h264->sps_len;

		body[i++] = 0x01;
		body[i++] = (h264->pps_len >> 8) & 0xFF;
		body[i++] = h264->pps_len & 0xFF;
		memcpy(&body[i], h264->pps + 4, h264->pps_len);
		i += h264->pps_len;

		//发送AVC视频同步包
		ret = rtmp_api_sendPacket(handle, RTMP_PACKET_TYPE_VIDEO, body, i, 0, false);
		if (ret != 0)
			RTMP_LOGE("send packet failed!\n");
	}
	else
	{
		ret = -1;
		RTMP_LOGE("first frame is not key frame!\n");
	}

	return ret;
}

int rtmp_api_sendSegmentFirst(RTMP *handle, const char *_data, unsigned int _size)
{
	int ret = 0;
	rtmp_h264_info nalus = { 0 };
	//读取h264中的NALU
	ret = read_H264_codec_info(_data, _size, &nalus);
	if (0 != ret)
		RTMP_LOGE("read h264 codec info failed!\n");


	//第一个视频帧必须需是avc视频同步帧
	ret = send_AVC_sequence_packet(handle, &nalus);
	if (0 != ret)
		RTMP_LOGE("send_AVC_sequence_packet failed!\n");

	return 0;
}

int rtmp_api_sendSegmentPacket(RTMP *handle, const char *_data, unsigned int _size, unsigned int _nTimeStamp, unsigned int frame_size,unsigned int is_new)
{
	if (_data == NULL && _size < 11 && frame_size <= 0)
		return false;

	int i = 0;
	int ret = 0;
	char *body = NULL;
	rtmp_h264_info nalus = { 0 };

	body = (char *)malloc(_size + 9);
	if (body == NULL)
	{
		RTMP_LOGE("malloc body failed!\n");
		return -1;
	}

	if (0 == ret && is_new)
	{
		//读取h264中的NALU
		ret = read_H264_codec_info(_data, _size, &nalus);
		if (0 != ret)
			RTMP_LOGE("read h264 codec info failed!\n");
	}

	if (0 == ret)
	{
		if (is_new)
		{
			//Frame type and codec Id
			if (nalus.is_key_frame)
				body[i++] = 0x17; // 1:Iframe    7:AVC 
			else
				body[i++] = 0x27; // 2:Pframe    7:AVC 

			//rtmp video packet
			body[i++] = 0x01; //AVCPacketType: AVC NALU 					  
			body[i++] = 0x00;//composition time: 0x000000
			body[i++] = 0x00;
			body[i++] = 0x00;

			//H264 to  AVC(这里默认一个I帧里面有且只有sps ，pps，sei等单元。一个p帧有且只有一个p帧单元)
			if (nalus.sps_num)
			{
				memcpy(&body[i], nalus.sps, nalus.sps_len + 4);
				i += nalus.sps_len + 4;
			}

			if (nalus.pps_num)
			{
				memcpy(&body[i], nalus.pps, nalus.pps_len + 4);
				i += nalus.pps_len + 4;
			}

			if (nalus.sei_num)
			{
				memcpy(&body[i], nalus.sei, nalus.sei_len + 4);
				i += nalus.sei_len + 4;
			}

			//I帧或者P帧数据
			if (nalus.data && nalus.data_len > 0)
			{
				body[i++] = nalus.data_len >> 24;
				body[i++] = nalus.data_len >> 16;
				body[i++] = nalus.data_len >> 8;
				body[i++] = nalus.data_len & 0xff;
				memcpy(&body[i], nalus.data, nalus.data_len);
				i += nalus.data_len + 4;
			}
		}
		else
		{
			//这里默认只有I帧才会分片，如果有p帧需要添加帧类型变量
			body[i++] = 0x17; // 2:Pframe    7:AVC 
			//rtmp video packet
			body[i++] = 0x01; //AVCPacketType: AVC NALU 					  
			body[i++] = 0x00;//composition time: 0x000000
			body[i++] = 0x00;
			body[i++] = 0x00;

			memcpy(&body[i], _data, _size);
			i += _size;
		}
	}

	if (0 == ret)
	{
		RTMP_LOGE("i=%d is_new=%d data_size=%d frame_size=%d time_stamp=%lld", i,is_new,_size, frame_size,_nTimeStamp);

		RTMPPacket packet;
		RTMPPacket_Reset(&packet);
		RTMPPacket_Alloc(&packet, _size);
		if (is_new)
		{
			packet.m_headerType = 0x00;//消息类型0
			packet.m_packetType = 0x09;//视频
			packet.m_nChannel = 0x04;
			packet.m_headerType = RTMP_PACKET_SIZE_LARGE;
			packet.m_nTimeStamp = _nTimeStamp;
			packet.m_nInfoField2 = handle->m_stream_id;
			packet.m_nBodySize = frame_size;
		}
		else
		{
			packet.m_headerType = 0x03;//消息类型1
		}
		memcpy(packet.m_body, body, i);
		ret = RTMP_SendPacket(handle, &packet, false);
		RTMPPacket_Free(&packet);
	}	

	if (body)
		free(body);

	return ret;
}

int rtmp_api_sendH264Packet(RTMP *handle, const char *_data, unsigned int _size, unsigned int  is_first_frame, unsigned int _nTimeStamp)
{
	if (_data == NULL && _size < 11)
		return false;

	int i = 0;
	int ret = 0;
	char *body = NULL;
	rtmp_h264_info nalus = { 0 };

	body = (char *)malloc(_size + 20);
	if (body == NULL)
	{
		RTMP_LOGE("malloc body failed!\n");
		return -1;
	}

	if (0 == ret)
	{
		//读取h264中的NALU
		ret = read_H264_codec_info(_data, _size, &nalus);
		if (0 != ret)
			RTMP_LOGE(" read h264 codec info failed!\n");
	}

	if (0 == ret && is_first_frame)
	{
		//第一个视频帧必须需是avc视频同步帧并且是I帧
		ret = send_AVC_sequence_packet(handle, &nalus);
		if (0 != ret)
			RTMP_LOGE("send_AVC_sequence_packet failed!\n");
	}

	if (0 == ret)
	{
		//Frame type and codec Id
		if (nalus.is_key_frame)
			body[i++] = 0x17; // 1:Iframe    7:AVC 
		else
			body[i++] = 0x27; // 2:Pframe    7:AVC 

		//rtmp video packet
		body[i++] = 0x01; //AVCPacketType: AVC NALU 					  
		body[i++] = 0x00;//composition time: 0x000000
		body[i++] = 0x00;
		body[i++] = 0x00;

		//H264 to  AVC(这里默认一个I帧里面有且只有sps ，pps，sei等单元。一个p帧有且只有一个p帧单元)
//		if (nalus.sps_num)
//		{
//			memcpy(&body[i], nalus.sps, nalus.sps_len + 4);
//			i += nalus.sps_len + 4;
//		}
//
//		if (nalus.pps_num)
//		{
//			memcpy(&body[i], nalus.pps, nalus.pps_len + 4);
//			i += nalus.pps_len + 4;
//		}
//
//		if (nalus.sei_num)
//		{
//			memcpy(&body[i], nalus.sei, nalus.sei_len + 4);
//			i += nalus.sei_len + 4;
//		}

		//I帧或者P帧数据
		if (nalus.data && nalus.data_len > 0)
		{
			body[i++] = nalus.data_len >> 24;
			body[i++] = nalus.data_len >> 16;
			body[i++] = nalus.data_len >> 8;
			body[i++] = nalus.data_len & 0xff;
			memcpy(&body[i], nalus.data, nalus.data_len);
			i += nalus.data_len;
		}
	}

	if (0 == ret)
	{
		//一帧一帧的发送
		ret = rtmp_api_sendPacket(handle, RTMP_PACKET_TYPE_VIDEO, body, i, _nTimeStamp, false);
		if (0 != ret)
			RTMP_LOGE("rtmp_api_sendPacket failed!\n");
	}

	if (body)
		free(body);

	return ret;
}

RTMP *rtmp_api_connect(const char *_url, int chunk_size)
{
	RTMP *handle = NULL;

	handle = RTMP_Alloc();
	if (NULL == handle)
	{
		RTMP_LOGE("RTMP_Alloc failed!\n");
		return NULL;
	}

	RTMP_Init(handle);

	if (RTMP_SetupURL(handle, (char *)_url) == false) {
		RTMP_Close(handle);
		RTMP_Free(handle);
		handle = NULL;
		return NULL;
	}

	RTMP_EnableWrite(handle);
	
	if (RTMP_Connect(handle, NULL) == false) {
		RTMP_Close(handle);
		RTMP_Free(handle);
		handle = NULL;
		return NULL;
	}

	if (RTMP_ConnectStream(handle, 0) == false) {
		RTMP_Close(handle);
		RTMP_Free(handle);
		handle = NULL;
		return NULL;
	}

	if (RTMP_IsConnected(handle) == false) {
		RTMP_Close(handle);
		RTMP_Free(handle);
		handle = NULL;
		return NULL;
	}

	if (rtmp_change_chunk_size(handle, chunk_size) == false)
	{
		RTMP_Close(handle);
		RTMP_Free(handle);
		handle = NULL;
		return NULL;
	}
	
	return handle;
}

void rtmp_api_close(RTMP *handle)
{
	if (handle != NULL) {
		RTMP_Close(handle);
		RTMP_Free(handle);
		handle = NULL;
	}
}
