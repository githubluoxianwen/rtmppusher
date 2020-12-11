#include "rtmp_queue.h"
#include "sys/sys_mem.h"
#include "util/rj_type.h"

static int is_full(rtmp_queue_t *head)
{
	if (head == NULL)
		return 0;

	return (head->front_id == (head->rear_id + 1) % head->queue_len)  ? 1 : 0;
}

static int is_empty(rtmp_queue_t *head)
{
	if (head == NULL)
		return 0;

	return head->front == head->rear ? 1 : 0;
}

int rtmp_queue_size(rtmp_queue_t *head)
{
	if (head == NULL)
		return 0;

	return head->node_num;
}


rtmp_queue_t *rtmp_create_queue(int queue_len, int node_type_size, RTMP_QUEUE_FREE_CB *free_cb)
{
	if (free_cb == NULL)
		return NULL;

	rtmp_queue_t *queue = NULL;
	queue = (rtmp_queue_t *)sys_malloc(sizeof(rtmp_queue_t));
	if (queue == NULL)
		return NULL;
	memset(queue, 0, sizeof(rtmp_queue_t));

	queue->node_num = 0;
	queue->node_type_size = node_type_size;
	queue->queue_len = queue_len;

	queue->array_queue = (void **)malloc(queue->queue_len * sizeof(void *));
	if (NULL == queue->array_queue)
	{
		sys_free(queue);
		return NULL;
	}
	memset(queue->array_queue, 0, queue->queue_len * sizeof(void *));

	int i = 0;
	for (i = 0; i < queue->queue_len; i++)
	{
		queue->array_queue[i] = (void *)sys_malloc(queue->node_type_size);
		if (queue->array_queue[i] == NULL)
		{
			for (; i >= 0; i--)
			{
				if (queue->array_queue[i])
					sys_free(queue->array_queue[i]);
				queue->array_queue[i] = NULL;
			}
			break;
		}
		memset(queue->array_queue[i], 0, queue->node_type_size);
	}

	if (i < queue->queue_len)
	{
		sys_free(queue->array_queue);
		sys_free(queue);
		return NULL;
	}

	queue->front = (void *)queue->array_queue[0];
	queue->front_id = 0;
	queue->rear = (void *)queue->array_queue[0];
	queue->rear_id = 0;
	queue->free_cb = free_cb;

	return queue;
}

void rtmp_destroy_queue(rtmp_queue_t *head)
{
	if (head == NULL)
		return;

	if (NULL == head || head->array_queue == NULL)
		return;

	int i = 0;
	for (i = 0; i < head->queue_len; i++)
	{
		if (head->array_queue[i])
		{
			if (head->free_cb)
				head->free_cb(head->array_queue[i]);

			sys_free(head->array_queue[i]);
		}
	}
	sys_free(head->array_queue);
	sys_free(head);
	head = NULL;

	return;
}

void *rtmp_queue_pop(rtmp_queue_t *head)
{
	if (head == NULL)
		return NULL;

	void *p_ret = NULL;

	if (is_empty(head))
		return  p_ret;
	else
	{
		p_ret = head->front;
		head->front_id = (head->front_id + 1) % head->queue_len;
		head->front = head->array_queue[head->front_id]; 
	}
	head->node_num--;
	return p_ret;
}

void *rtmp_queue_push(rtmp_queue_t *head)
{
	if (head == NULL)
		return NULL;

	void *p_ret = NULL;

	if (is_full(head))
		return p_ret;
	else
	{
		p_ret = head->rear;
		head->rear_id = (head->rear_id + 1) % head->queue_len;
		head->rear = head->array_queue[head->rear_id]; 
	}

	head->node_num++;
	return p_ret;
}