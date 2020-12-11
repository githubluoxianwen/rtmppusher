#pragma once
// Created:  2018 / 05 / 22
// @file	 rtmp_queue.h
// @brief	 ���У���֧��ģ�壬��֧�ֶ��̰߳�ȫ
// @author   LXW
// @version  0.1
// @warning  û�о���

#ifndef __RTMP_QUEUE__
#define __RTMP_QUEUE__

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

typedef void(RTMP_QUEUE_FREE_CB)(void *node);

typedef struct
{
	int node_num;                       //���е�ǰ����ڵ����
	int queue_len;                      //�������нڵ����
	int node_type_size;                 //�ڵ��������ʹ�С 
	void **array_queue;                 //���нڵ��ڴ�ָ������
	void *front;                        //����ͷָ��
	int front_id;                       //ͷ�������е�id
	void *rear;                         //����βָ��
	int rear_id;                        //β�������е�id
	RTMP_QUEUE_FREE_CB *free_cb;        //�ͷŽڵ����û������ڴ�ص�
}rtmp_queue_t;

/*
/// @brief                          �����������
/// @param [in] queue_len           ���нڵ����
/// @param [in] node_type_size      ���нڵ��������͵Ĵ�С
/// @param [in] free_cb             ���нڵ��ͷ��ڴ�ص�
/// @return rtmp_queue_t *          ���ض��о����������ʧ���򷵻�NULL��	
*/
rtmp_queue_t *rtmp_create_queue(int queue_len, int node_type_size, RTMP_QUEUE_FREE_CB *free_cb);

/*
/// @brief                     �����������
/// @param [in] head           ���о��
*/
void rtmp_destroy_queue(rtmp_queue_t *head);

/*
/// @brief                     �ڵ����ݳ�����
/// @param [in] head           ���о��
/// @return void *             ���س����еĽڵ��ڴ��ַ
*/
void *rtmp_queue_pop(rtmp_queue_t *head);

/*
/// @brief                     �ڵ����������
/// @param [in] head           ���о��
/// @return void *             ��������еĽڵ��ڴ��ַ
*/
void *rtmp_queue_push(rtmp_queue_t *head);

/*
/// @brief                     ��ȡ���е�ǰ�ڵ����
/// @param [in] head           ���о��
/// @return int                ���ض��е�ǰ�ڵ����
*/
int rtmp_queue_size(rtmp_queue_t *head);
#endif //__RTMP_QUEUE__