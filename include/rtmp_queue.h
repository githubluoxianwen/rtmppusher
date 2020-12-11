#pragma once
// Created:  2018 / 05 / 22
// @file	 rtmp_queue.h
// @brief	 队列，不支持模板，不支持多线程安全
// @author   LXW
// @version  0.1
// @warning  没有警告

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
	int node_num;                       //队列当前缓冲节点个数
	int queue_len;                      //队列所有节点个数
	int node_type_size;                 //节点数据类型大小 
	void **array_queue;                 //队列节点内存指针数组
	void *front;                        //队列头指针
	int front_id;                       //头在数组中的id
	void *rear;                         //队列尾指针
	int rear_id;                        //尾在数组中的id
	RTMP_QUEUE_FREE_CB *free_cb;        //释放节点中用户数据内存回调
}rtmp_queue_t;

/*
/// @brief                          创建数组队列
/// @param [in] queue_len           队列节点个数
/// @param [in] node_type_size      队列节点数据类型的大小
/// @param [in] free_cb             队列节点释放内存回调
/// @return rtmp_queue_t *          返回队列句柄，若创建失败则返回NULL。	
*/
rtmp_queue_t *rtmp_create_queue(int queue_len, int node_type_size, RTMP_QUEUE_FREE_CB *free_cb);

/*
/// @brief                     销毁数组队列
/// @param [in] head           队列句柄
*/
void rtmp_destroy_queue(rtmp_queue_t *head);

/*
/// @brief                     节点数据出队列
/// @param [in] head           队列句柄
/// @return void *             返回出队列的节点内存地址
*/
void *rtmp_queue_pop(rtmp_queue_t *head);

/*
/// @brief                     节点数据入队列
/// @param [in] head           队列句柄
/// @return void *             返回入队列的节点内存地址
*/
void *rtmp_queue_push(rtmp_queue_t *head);

/*
/// @brief                     获取队列当前节点个数
/// @param [in] head           队列句柄
/// @return int                返回队列当前节点个数
*/
int rtmp_queue_size(rtmp_queue_t *head);
#endif //__RTMP_QUEUE__