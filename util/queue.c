/* ========================================================
 *   Copyright (C) 2014 All rights reserved.
 *   
 *   filename : queue.c
 *   author   : liuzhiqiang01@baidu.com
 *   date     : 2014-12-24
 *   info     : implementation of simple quene
 * ======================================================== */

#include <stdlib.h>
#include <string.h>
#include "queue.h"

Queue * q_create(int size){
    if (size < 2)
        return NULL;
    Queue * q = (Queue*)malloc(sizeof(Queue));
    if (!q)
        return NULL;
    q->data = (int*)malloc(sizeof(int) * size);
    memset(q->data, 0, sizeof(int) * size);
    q->size = size;
    q->head = q->tail = 0;
    q->flag = 0;
    return q;
}

int q_add(Queue * q, int l){
    // queue is not aviliable
    if (!q)
        return -1;
    // queue is full 
    // can not add futher
    if (1 == q->flag)
        return -1;

    // queue is not full
    q->data[q->tail] = l;
    q->tail += 1;
    if (q->tail >= q->size){
        q->tail -= q->size;
    }
    // queue is full after add
    if (q->tail == q->head){
        q->flag = 1;
    }
    // queue is not full or empty
    else{
        q->flag = 2;
    }
    return 0;
}

int q_pop(Queue * q, int *ol){
    // q or ol is not aviliable
    if (!q || !ol)
        return -1;
    // queue is empty
    if (0 == q->flag)
        return -1;
    // queue is not empty
    *ol = q->data[q->head];
    q->head += 1;
    if (q->head >= q->size){
        q->head -= q->size;
    }
    // queue is empty after pop
    if (q->head == q->tail){
        q->flag = 0;
    }
    // queue is not empty or full
    else{
        q->flag = 2;
    }
    return 0;
}

void q_free(Queue * q){
    if (q){
        if (q->data){
            free(q->data);
            q->data = NULL;
        }
        free(q);
    }
}

int     q_empty(Queue * q){
    return q->flag == 0 ? 1 : 0;
}
int     q_full(Queue * q){
    return q->flag == 1 ? 1 : 0;
}



void    q_clear(Queue * q){
    q->head = q->tail = 0;
    q->flag = 0;
}

////int main(){
////    int a[10] = {1,2,3,4,5,6,7,8,9,10};
////    int i = 0;
////    int ol = 0;
////    int f = 0;
////    Queue * q = q_create(2);
////    for (; i < 7; i++){
////        q_add(q, a[i]);
////    }
////    while (q_empty(q) != 1){
////        f = q_pop(q, &ol);
////        if (f == 0)
////            printf("%d\n", ol);
////    }
////    for(; i < 10; i++){
////        q_add(q, a[i]);
////    }
////    while (q_empty(q) != 1){
////        f = q_pop(q, &ol);
////        if (f == 0)
////            printf("%d\n", ol);
////    }
////    return 0;
////}
