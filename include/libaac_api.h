#ifndef __LIBAAC_API__
#define __LIBAAC_API__

#include "libaac/frame.h"
#include "libaac/faaccfg.h"
#include "librtmp_api.h"
#include <stdio.h>

/*
RTMP packet
	packet type
	time stamp

FLV Tag
	avc header
	aac header

AVC
	avc sequence header
	avc data

AAC
	aac sequence header
		aac data
		adts data
*/

typedef enum
{
	AAC_MAIN = 1,   //
	AAC_LC,     //Low Complexity
	AAC_SSR,    //Scalable Sample Rate
	AAC_LTP     //Long Term Prediction
}AUDIO_OBJ_E;

typedef enum
{
	HZ_96000 = 0,
	HZ_88200,
	HZ_64000,
	HZ_48000,
	HZ_44100,
	HZ_32000,
	HZ_24000,
	HZ_22050,
	HZ_16000,
	HZ_12000,
	HZ_11025,
	HZ_8000,
	HZ_7350,
	HZ_END
}MPEG4_SAMPLE_RATE_E;

typedef enum
{
	TRACK_0 = 0, //Defined in AOT Specifc Config
	TRACK_1,     //1 channel: front-center
	TRACK_2,     //2 channels: front-left, front-right
	TRACK_3,     //3 channels: front-center, front-left, front-right
	TRACK_4,     //4 channels: front-center, front-left, front-right, back-center
	TRACK_5,     //5 channels: front-center, front-left, front-right, back-left, back-right
	TRACK_6,     //6 channels: front-center, front-left, front-right, back-left, back-right, LFE-channel
	TRACK_7,     //8 channels: front-center, front-left, front-right, side-left, side-right, back-left, back-right, LFE-channel
	TRACK_END
}MPEG4_TRACK_E;

static int sample_rate_table[] = {96000,88200,64000,48000,44100,32000,24000,22050,16000,12000,11025,8000,7350};

typedef struct
{
	faacEncHandle									encoder_handle;
	faacEncConfigurationPtr							configuration;
	unsigned char*									PCM_buffer;
	unsigned int									PCM_bit_size;
	int												PCM_buffer_size;
	unsigned long									sample_rate;
	unsigned int									channels;
	unsigned long									input_samples;
	unsigned long									max_output_bytes;
	unsigned char*									g711a_buffer;
	int												g711a_len;
	unsigned int                                    last_time_sample;
}AAC;


AAC *aac_api_create(unsigned int sample, unsigned int bit_wide, unsigned int track);
int aac_api_destroy(AAC *aac_handle);
int aac_api_sendAACPacket(AAC *aac_handle, RTMP *rtmp_handle, char *_data, unsigned int _size, unsigned int encoder_type, unsigned int is_first_data, unsigned int time_delta, unsigned int *_nTimeStamp);
#endif //__LIBAAC_API__
