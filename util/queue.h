/* ========================================================
 *   Copyright (C) 2014 All rights reserved.
 *   
 *   filename : queue.h
 *   author   : liuzhiqiang01@baidu.com
 *   date     : 2014-12-24
 *   info     : FIFO Cycle Queue
 * ======================================================== */

#ifndef _QUEUE_H
#define _QUEUE_H

typedef struct _queue{
    int * data;    /* data space */
    int size;      /* space size */
    int head;      /* head index */
    int tail;      /* tail index */
    int flag;      /* 0:empty, 1:full, 2:normal */
} Queue;


Queue * q_create(int size);
int     q_add(Queue * q, int l);
int     q_pop(Queue * q, int * ol);
int     q_empty(Queue * q);
int     q_full(Queue * q);
void    q_clear(Queue * q);
void    q_free(Queue * q);


#endif //QUEUE_H

