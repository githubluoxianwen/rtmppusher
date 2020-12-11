#ifndef __RTMP_SERVER__
#define __RTMP_SERVER__

#include "util/rj_type.h"
#include "pub/pub_define.h"
#include "pub/rj_ndp.h"
#include "pub/rj_frame.h"
#include "sys/sys_pthread.h"
#include "sys/sys_time.h"
#include "sys/sys_mem.h"
#include "util/logger.h"
#include "librtmp_api.h"
#include "libaac_api.h"
#include "util/rj_queue.h"
#include "rtmp_queue.h"

#define RTMP_MAX_CLIENT_NUM 32

#define RTMP_SESSIONID_LEN 128
#define RTMP_IP_LEN 16

#define RTMP_HOST_NAME_LEN 256
#define RTMP_APPLICATION_LEN 512
/*注意结构体对齐*/

typedef enum
{
	RTMP_PROTOCOL = 0,           //rtmp协议
	RTMPS_PROTOCOL = 1           //rtmps协议
}RTMP_PROC_TYPE;

typedef enum
{
	RTMP_PUSH_RECORD = 0,        //推录像
	RTMP_PUSH_LIVE               //推实时流
}RTMP_WORK_MODE_E;

typedef enum
{
	RTMP_STREAM_DISCONNECT = 0,  //未连接
	RTMP_STREAM_CONNECTED,       //已连接
	RTMP_UPLOAD_SUCCESS,         //录像全部上传成功
	RTMP_OVERDUE_ID,             //无效ID
	RTMP_RECORD_SEEKING,         //录像拖动
}RTMP_STATUS_E;

typedef struct
{
	uint32 is_Iframe;            //I帧
	uint32 encode_type;          //编码类型
	uint32 curr_frame_size;      //片所属的当前帧的长度
	uint32 time_stamp;           //时间戳
	uint32 frame_rate;           //帧率   //NVR中目前没有使用
	uint32 frame_no;             //帧号 //NVR中目前没有使用
	uint32 image_height;         //分辨率高
	uint32 image_width;          //分辨率宽
	uint32 is_first_video;       //是否为第一帧视频
}rtmp_video_t;

typedef struct
{
	uint32 time_stamp;           //时间戳
	uint32 track;                //音轨（左右声道）
	uint32 frame_size;           //音频帧大小
	uint32 bit_wide;             //音频采样位宽
	uint32 sample;               //音频采样率
	uint32 encode_type;          //音频编码类型
	uint32 is_first_audio;       //是否为第一帧音频
}rtmp_audio_t;

typedef struct
{
	uint32 streamId;             //流id
	uint32 cameraNo;             //设备号，//NVR中暂时没有使用
	uint32 stream_type;          //流类型需要计算后才能使用 rj_frame_t.type  
	uint32 frame_type;           //帧类型音频 视频等，具体计算后的类型rj_frame_type_e

	uint32 has_h264_header;      //是否携带h264头
	char *data;                  //数据指针
	uint32 data_len;             //数据长度
	char *frame_buff;            //一帧数据缓冲区指针
	uint32 buff_size;            //缓冲大小
	uint32 buff_len;             //缓冲区中数据长度

	union {
		rtmp_audio_t audio;
		rtmp_video_t video;
	}av_attr;
}rtmp_av_data_t;


typedef struct
{
	uint16 task_id;               //读取录像任务id
	uint32 state;                 //推录像流状态 RTMP_STATUS_E
	int64  start_time;            //读取录像的开始时间
	int64  end_time;              //读取录像的结束时间
	
	uint32 freeze_flg;			  //线程冻结标识（处于睡眠状态）
	int thread_flag;              //线程活动标识
	sys_thread_t *thread_hd;      //推送录像流线程句柄
	
}rtmp_record_t;

typedef struct
{
	char sessionId[RTMP_SESSIONID_LEN];         //ABB客户端唯一id

	char *url;                                  //客户端流地址
	int protocol;                               //连接协议0:rtmp 1:rtmps
	char ip[RTMP_IP_LEN];                       //客户端IP
	char host_name[RTMP_HOST_NAME_LEN];         //主机名
	uint32	port;                               //客户端发送端口
	char application[RTMP_APPLICATION_LEN];     //应用名

	uint32 client_id;            //客户号
	uint32 device_id;            //设备号
	uint32 channel_id;           //通道号
	uint32 stream_id;            //流号
	uint32 is_online;            //是否在线
	uint32 reconnect;            //客户端是否需要重连
	uint32 reconnect_count;      //客户端重连次数

	uint32 work_mode;            //工作模式 1-推送实时流 0-推送录像流 RTMP_WORK_MODE_E
	rtmp_record_t record;        //录像相关参数
	
	uint32 send_type;            //0-按数据包发送，1-按照一帧一帧发送
	uint32 video_enable;         //是否发送视频
	uint32 first_video_flag;     //是否为第一帧视频
	uint32 audio_enable;         //是否发送音频
	int32  first_audio_flag;     //是否为第一帧音频

	int64 last_audio_time_stamp;
	int64 last_video_time_stamp;

	uint32 time_sample;          //音视频同步时间戳
	uint32 time_delta;           //每帧音视频数据的时间增量
	uint32 video_standard_delta; //视频帧的标准时间增量，由帧率决定
	uint32 audio_standard_delta; //音频帧的标准时间增量，由采样率和帧大小决定
	uint32 pre_audio_delta;
	uint32 pre_video_delta;

	rtmp_av_data_t av_buff;      //帧信息缓冲
	uint32 curr_segm_count;      //当前帧的数据片计数。一帧片数达到一定值后读队列线程需要睡10ms

	RTMP *rtmp_handle;           //rtmp库操作句柄
	int chunk_size;              //rtmp分片大小
	AAC *aac_handle;             //aac编码器句柄
	uint32 wait_Iframe;          //队列满时，需等待I帧                 
	rtmp_queue_t *queue;         //缓冲队列
	sys_mutex_t *queue_mutex;    //缓冲队列锁

	sys_thread_t *send_thread_hd; //rtmp发送流服务线程句柄
	int send_thread_flag;         //线程活动标识
	void *resv;                   //保留字段
}rtmp_client_t;

typedef struct
{
	uint32 client_max_num;       //最大的客户连接数
	uint32 client_num;           //客户端个数(配置文件中客户端数)
	uint32 connect_num;          //已连接的客户端数
	uint32 reconnect_num;        //需要重连的客户端数
	rtmp_client_t *clients[RTMP_MAX_CLIENT_NUM];           //客户端信息
}rtmp_client_manager_t;

typedef struct
{
	uint32 init;                           //初始化标识
	sys_thread_t *server_hd;               //rtmp服务线程句柄
	int server_flag;                       //线程活动标识
	sys_thread_t *disptch_hd;              //rtmp发生线程句柄
	int disptch_flag;                      //线程活动标识
	sys_mutex_t *client_mutex;             //客户端信息锁
	rtmp_client_manager_t client_manager;  //客户端管理者
}rtmp_rtmpserver_t;

typedef struct
{      
	int enable;      //使能配置
	int online;      //是否在线
	int client_id;   //客户id
	char url[256];   //url地址
	int device_id;   //设备号
	int channl_id;   //通道号
	int record;      //是否为录像流
	int stream;      //主子码流
	int h264;        //视频是否是h264编码
	int audio;       //音频使能
	int g711a;       //音频是否是g711a编码
	int sample;      //音频采样率
	int wide;        //音频采样位宽
	int track;       //音频轨道数
}rtmp_client_cnf;


//rtmp服务开启
int rtmp_server_start();
//rtmp服务停止
int rtmp_server_stop();
//rtmp服务分发流
int rtmp_server_disptch(int stream_flag, int channel, rj_ndp_pk_t *ndp, char *data);



//获取指定id客户端连接状态
int rtmp_get_status(char *sessionId);
//创建推实时流流客户端,紧接着要调用rtmp_get_status来获取连接是否成功，如果连接不成功需要调rtmp_destroy_publisher来释放资源
int rtmp_create_publisher(char *sessionId, int channel, int stream, char *url);
//创建推录像流客户端，紧接着要调用rtmp_get_status来获取连接是否成功，如果连接不成功需要调rtmp_destroy_publisher来释放资源
int rtmp_create_playback(char *sessionId, int channel, int stream, int64 s_time, int64 e_time, char *url);
//销毁推流客户端
int rtmp_destroy_publisher(char *sessionId);
//开始或停止指定客户端视频
int rtmp_set_video(char *sessionId, int enable);
//开始或停止指定客户端音频
int rtmp_set_audio(char *sessionId, int enable);
//回放拖动
int rtmp_seek_playback(char *sessionId, int64 seek_time);

#endif //__RTMP_SERVER__