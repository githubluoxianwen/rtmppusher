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
/*ע��ṹ�����*/

typedef enum
{
	RTMP_PROTOCOL = 0,           //rtmpЭ��
	RTMPS_PROTOCOL = 1           //rtmpsЭ��
}RTMP_PROC_TYPE;

typedef enum
{
	RTMP_PUSH_RECORD = 0,        //��¼��
	RTMP_PUSH_LIVE               //��ʵʱ��
}RTMP_WORK_MODE_E;

typedef enum
{
	RTMP_STREAM_DISCONNECT = 0,  //δ����
	RTMP_STREAM_CONNECTED,       //������
	RTMP_UPLOAD_SUCCESS,         //¼��ȫ���ϴ��ɹ�
	RTMP_OVERDUE_ID,             //��ЧID
	RTMP_RECORD_SEEKING,         //¼���϶�
}RTMP_STATUS_E;

typedef struct
{
	uint32 is_Iframe;            //I֡
	uint32 encode_type;          //��������
	uint32 curr_frame_size;      //Ƭ�����ĵ�ǰ֡�ĳ���
	uint32 time_stamp;           //ʱ���
	uint32 frame_rate;           //֡��   //NVR��Ŀǰû��ʹ��
	uint32 frame_no;             //֡�� //NVR��Ŀǰû��ʹ��
	uint32 image_height;         //�ֱ��ʸ�
	uint32 image_width;          //�ֱ��ʿ�
	uint32 is_first_video;       //�Ƿ�Ϊ��һ֡��Ƶ
}rtmp_video_t;

typedef struct
{
	uint32 time_stamp;           //ʱ���
	uint32 track;                //���죨����������
	uint32 frame_size;           //��Ƶ֡��С
	uint32 bit_wide;             //��Ƶ����λ��
	uint32 sample;               //��Ƶ������
	uint32 encode_type;          //��Ƶ��������
	uint32 is_first_audio;       //�Ƿ�Ϊ��һ֡��Ƶ
}rtmp_audio_t;

typedef struct
{
	uint32 streamId;             //��id
	uint32 cameraNo;             //�豸�ţ�//NVR����ʱû��ʹ��
	uint32 stream_type;          //��������Ҫ��������ʹ�� rj_frame_t.type  
	uint32 frame_type;           //֡������Ƶ ��Ƶ�ȣ��������������rj_frame_type_e

	uint32 has_h264_header;      //�Ƿ�Я��h264ͷ
	char *data;                  //����ָ��
	uint32 data_len;             //���ݳ���
	char *frame_buff;            //һ֡���ݻ�����ָ��
	uint32 buff_size;            //�����С
	uint32 buff_len;             //�����������ݳ���

	union {
		rtmp_audio_t audio;
		rtmp_video_t video;
	}av_attr;
}rtmp_av_data_t;


typedef struct
{
	uint16 task_id;               //��ȡ¼������id
	uint32 state;                 //��¼����״̬ RTMP_STATUS_E
	int64  start_time;            //��ȡ¼��Ŀ�ʼʱ��
	int64  end_time;              //��ȡ¼��Ľ���ʱ��
	
	uint32 freeze_flg;			  //�̶߳����ʶ������˯��״̬��
	int thread_flag;              //�̻߳��ʶ
	sys_thread_t *thread_hd;      //����¼�����߳̾��
	
}rtmp_record_t;

typedef struct
{
	char sessionId[RTMP_SESSIONID_LEN];         //ABB�ͻ���Ψһid

	char *url;                                  //�ͻ�������ַ
	int protocol;                               //����Э��0:rtmp 1:rtmps
	char ip[RTMP_IP_LEN];                       //�ͻ���IP
	char host_name[RTMP_HOST_NAME_LEN];         //������
	uint32	port;                               //�ͻ��˷��Ͷ˿�
	char application[RTMP_APPLICATION_LEN];     //Ӧ����

	uint32 client_id;            //�ͻ���
	uint32 device_id;            //�豸��
	uint32 channel_id;           //ͨ����
	uint32 stream_id;            //����
	uint32 is_online;            //�Ƿ�����
	uint32 reconnect;            //�ͻ����Ƿ���Ҫ����
	uint32 reconnect_count;      //�ͻ�����������

	uint32 work_mode;            //����ģʽ 1-����ʵʱ�� 0-����¼���� RTMP_WORK_MODE_E
	rtmp_record_t record;        //¼����ز���
	
	uint32 send_type;            //0-�����ݰ����ͣ�1-����һ֡һ֡����
	uint32 video_enable;         //�Ƿ�����Ƶ
	uint32 first_video_flag;     //�Ƿ�Ϊ��һ֡��Ƶ
	uint32 audio_enable;         //�Ƿ�����Ƶ
	int32  first_audio_flag;     //�Ƿ�Ϊ��һ֡��Ƶ

	int64 last_audio_time_stamp;
	int64 last_video_time_stamp;

	uint32 time_sample;          //����Ƶͬ��ʱ���
	uint32 time_delta;           //ÿ֡����Ƶ���ݵ�ʱ������
	uint32 video_standard_delta; //��Ƶ֡�ı�׼ʱ����������֡�ʾ���
	uint32 audio_standard_delta; //��Ƶ֡�ı�׼ʱ���������ɲ����ʺ�֡��С����
	uint32 pre_audio_delta;
	uint32 pre_video_delta;

	rtmp_av_data_t av_buff;      //֡��Ϣ����
	uint32 curr_segm_count;      //��ǰ֡������Ƭ������һ֡Ƭ���ﵽһ��ֵ��������߳���Ҫ˯10ms

	RTMP *rtmp_handle;           //rtmp��������
	int chunk_size;              //rtmp��Ƭ��С
	AAC *aac_handle;             //aac���������
	uint32 wait_Iframe;          //������ʱ����ȴ�I֡                 
	rtmp_queue_t *queue;         //�������
	sys_mutex_t *queue_mutex;    //���������

	sys_thread_t *send_thread_hd; //rtmp�����������߳̾��
	int send_thread_flag;         //�̻߳��ʶ
	void *resv;                   //�����ֶ�
}rtmp_client_t;

typedef struct
{
	uint32 client_max_num;       //���Ŀͻ�������
	uint32 client_num;           //�ͻ��˸���(�����ļ��пͻ�����)
	uint32 connect_num;          //�����ӵĿͻ�����
	uint32 reconnect_num;        //��Ҫ�����Ŀͻ�����
	rtmp_client_t *clients[RTMP_MAX_CLIENT_NUM];           //�ͻ�����Ϣ
}rtmp_client_manager_t;

typedef struct
{
	uint32 init;                           //��ʼ����ʶ
	sys_thread_t *server_hd;               //rtmp�����߳̾��
	int server_flag;                       //�̻߳��ʶ
	sys_thread_t *disptch_hd;              //rtmp�����߳̾��
	int disptch_flag;                      //�̻߳��ʶ
	sys_mutex_t *client_mutex;             //�ͻ�����Ϣ��
	rtmp_client_manager_t client_manager;  //�ͻ��˹�����
}rtmp_rtmpserver_t;

typedef struct
{      
	int enable;      //ʹ������
	int online;      //�Ƿ�����
	int client_id;   //�ͻ�id
	char url[256];   //url��ַ
	int device_id;   //�豸��
	int channl_id;   //ͨ����
	int record;      //�Ƿ�Ϊ¼����
	int stream;      //��������
	int h264;        //��Ƶ�Ƿ���h264����
	int audio;       //��Ƶʹ��
	int g711a;       //��Ƶ�Ƿ���g711a����
	int sample;      //��Ƶ������
	int wide;        //��Ƶ����λ��
	int track;       //��Ƶ�����
}rtmp_client_cnf;


//rtmp������
int rtmp_server_start();
//rtmp����ֹͣ
int rtmp_server_stop();
//rtmp����ַ���
int rtmp_server_disptch(int stream_flag, int channel, rj_ndp_pk_t *ndp, char *data);



//��ȡָ��id�ͻ�������״̬
int rtmp_get_status(char *sessionId);
//������ʵʱ�����ͻ���,������Ҫ����rtmp_get_status����ȡ�����Ƿ�ɹ���������Ӳ��ɹ���Ҫ��rtmp_destroy_publisher���ͷ���Դ
int rtmp_create_publisher(char *sessionId, int channel, int stream, char *url);
//������¼�����ͻ��ˣ�������Ҫ����rtmp_get_status����ȡ�����Ƿ�ɹ���������Ӳ��ɹ���Ҫ��rtmp_destroy_publisher���ͷ���Դ
int rtmp_create_playback(char *sessionId, int channel, int stream, int64 s_time, int64 e_time, char *url);
//���������ͻ���
int rtmp_destroy_publisher(char *sessionId);
//��ʼ��ָֹͣ���ͻ�����Ƶ
int rtmp_set_video(char *sessionId, int enable);
//��ʼ��ָֹͣ���ͻ�����Ƶ
int rtmp_set_audio(char *sessionId, int enable);
//�ط��϶�
int rtmp_seek_playback(char *sessionId, int64 seek_time);

#endif //__RTMP_SERVER__