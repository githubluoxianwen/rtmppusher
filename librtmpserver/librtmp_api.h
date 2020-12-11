#ifndef __LIBRTMP_API__
#define __LIBRTMP_API__

#include "librtmp/rtmp.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "sys/sys_time.h"
#include <time.h>
#ifdef __RJ_LINUX__
#include <sys/time.h>
#include <arpa/inet.h>
#endif


#ifdef __RJ_LINUX__
#define RTMP_TIME struct timeval tv; struct tm *ptm;gettimeofday(&tv, NULL);ptm = localtime(&tv.tv_sec);
#define __TIME_MS__ ptm->tm_year + 1900,ptm->tm_mon + 1,ptm->tm_mday,ptm->tm_hour,ptm->tm_min,ptm->tm_sec,tv.tv_usec
#define RTMP_LOGE(format,...) do{RTMP_TIME; printf("\033[1m\033[31m%d-%02d-%02d %02d:%02d:%02d.%d [RTMP:Error:%s:%s:%d]"#format"\033[0m\n",__TIME_MS__,__FILE__,__FUNCTION__,__LINE__,##__VA_ARGS__);}while(0)
#define RTMP_LOGD(format,...) do{RTMP_TIME; printf("\033[36m%d-%02d-%02d %02d:%02d:%02d.%d [RTMP:Debug:%s:%s:%d]"#format"\033[0m\n",__TIME_MS__,__FILE__,__FUNCTION__,__LINE__,##__VA_ARGS__);}while(0)
#define RTMP_LOGI(format,...) do{RTMP_TIME; printf("\033[32m%d-%02d-%02d %02d:%02d:%02d.%d [RTMP:Info:%s:%s:%d]"#format"\033[0m\n",__TIME_MS__,__FILE__,__FUNCTION__,__LINE__,##__VA_ARGS__);}while(0)
#define RTMP_LOGW(format,...) do{RTMP_TIME; printf("\033[1m\033[33m%d-%02d-%02d %02d:%02d:%02d.%d [RTMP:Warn:%s:%s:%d]"#format"\033[0m\n",__TIME_MS__,__FILE__,__FUNCTION__,__LINE__,##__VA_ARGS__);}while(0)
#else define __RJ_WIN__
#define RTMP_LOGE(format,...) do{printf("[RTMP:Error:%s:%d]:",__FUNCTION__,__LINE__);printf(format,##__VA_ARGS__);}while(0)
#define RTMP_LOGD(format,...) do{printf("[RTMP:Debug:%s:%d]:",__FUNCTION__,__LINE__);printf(format,##__VA_ARGS__);}while(0)
#define RTMP_LOGI(format,...) do{printf("[RTMP:Info:%s:%d]:",__FUNCTION__,__LINE__);printf(format,##__VA_ARGS__);}while(0)
#define RTMP_LOGW(format,...) do{printf("[RTMP:Warn:%s:%d]:",__FUNCTION__,__LINE__);printf(format,##__VA_ARGS__);}while(0)
#endif

#define    RTMP_SPS_LEN          256
#define    RTMP_PPS_LEN          128
#define    RTMP_SEI_LEN          128

typedef struct
{
	int is_key_frame;
	char *data;
	int data_len;
	char pps[RTMP_PPS_LEN];             ///< pps 其中前四个字节pps的长度，方便从h264转AVC
	int pps_len;                        ///< pps 长度, 不包括前四个字的长度，方便转AVC
	int pps_num;                        ///< pps 个数
	char sps[RTMP_SPS_LEN];             ///< sps 其中前四个字节sps的长度，方便从h264转AVC
	int sps_len;                        ///< sps 长度, 不包括前四个字的长度，方便转AVC
	int sps_num;                        ///< sps 个数
	char sei[RTMP_SEI_LEN];             //h264里面的SEI单元，一般出现在关键帧帧里面并且在pps之后在I帧数据之前
	int sei_len;                        
	int sei_num;
}rtmp_h264_info;


RTMP *rtmp_api_connect(const char *_url, int chunk_size);
int rtmp_change_chunk_size(RTMP *r, int size);
void rtmp_api_close(RTMP *handle);
int rtmp_api_sendSegmentFirst(RTMP *handle, const char *_data, unsigned int _size);
int rtmp_api_sendPacket(RTMP *handle, unsigned int _nPacketType, const char *_data, unsigned int _size, unsigned int _nTimestamp, bool p_bQueue);
int rtmp_api_sendSegmentPacket(RTMP *handle, const char *_data, unsigned int _size, unsigned int _nTimeStamp, unsigned int frame_size, unsigned int is_new);
int rtmp_api_sendH264Packet(RTMP *handle, const char *_data, unsigned int _size, unsigned int is_first_frame, unsigned int _nTimeStamp);

#endif //__LIBRTMP_API__
