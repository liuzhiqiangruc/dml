/* ========================================================
 *   Copyright (C) 2014 All rights reserved.
 *   
 *   filename : heap.c
 *   author   : liuzhiqiang01@baidu.com
 *   date     : 2014-09-19
 *   info     : implementation of heap
 * ======================================================== */

#include <stdlib.h>
#include <string.h>
#include "heap.h"

/* -------------------------------------
 * Min Top Heap compare function
 * ------------------------------------- */
int default_heap_cmp_fn(int* m, int* n){
    int i = *m - *n;
    return i > 0 ? 1 : (i < 0 ? -1 : 0);
}


/* ----------------------------------
 * Create and Return a Heap Structor
 * It is a min top heap by default
 * ---------------------------------- */
Heap * heap_create(int size){
    if (size < 2){
        return NULL;

    }

    Heap * hp = (Heap*)malloc(sizeof(Heap));
    hp->len = 0;
    hp->size = size;
    hp->data = (void**)malloc(sizeof(void*) * size);
    memset(hp->data,0,sizeof(void*) * size);
    hp->heap_cmp_fn = (HEAP_CMP_FN)&default_heap_cmp_fn;
    hp->heap_item_free_fn = NULL;

    return hp;
}

/* -------------------------------------------
 * Make the <Min Top> Heap for Real shift down
 * ------------------------------------------- */

#define HEAP_ADJUST(hp,l) do {                                             \
    int lt = l;                                                            \
    int i  = lt + lt + 1;                                                  \
    int r  = hp->len - 1;                                                  \
    void * t = hp->data[lt];                                               \
    do {                                                                   \
        if ((i<r) && (hp->heap_cmp_fn(hp->data[i], hp->data[i+1]) > 0)){   \
            i += 1;                                                        \
        }                                                                  \
        if (hp->heap_cmp_fn(t,hp->data[i]) <= 0){                          \
            break;                                                         \
        }                                                                  \
        hp->data[lt] = hp->data[i];                                        \
        lt = i;                                                            \
        i += i + 1;                                                        \
    }while (i <= r);                                                       \
    hp->data[lt] = t;                                                      \
} while (0);                                                               \
                                                                           

/* ----------------------------------
 * Add an Element into the Heap 
 * ---------------------------------- */
void heap_add(Heap * hp, void * pdata){
    if (hp->len < hp->size){
        hp->data[hp->len] = pdata;
        hp->len += 1;
        if (hp->len > 1){
            int l = (hp->len >> 1) - 1;
            while(l > 0){
                HEAP_ADJUST(hp, l);
                l = (l - 1) >> 1;
            }
            HEAP_ADJUST(hp, l);
        }
    }
    else{
        int size = hp->size + hp->size / 2;
        void ** data = (void**)malloc(sizeof(void*) * size);
        memset(data,0,sizeof(void*) * size);
        memmove(data, hp->data, sizeof(void*) * hp->len);
        free(hp->data); hp->data = data; hp->size = size;
        heap_add(hp, pdata);
    }
}


/* ----------------------------------
 * Remove an Element from the Heap 
 * ---------------------------------- */
void heap_remove(Heap * hp, void * pdata){
    int index = 0;
    for (; index < hp->len; index++){
        if (hp->heap_cmp_fn(pdata,hp->data[index]) == 0){
            break;
        }
    }

    // element find in heap 
    if (index >= 0 && index < hp->len){
        // free the data space if needed
        if (NULL != hp->heap_item_free_fn){
            hp->heap_item_free_fn(hp->data[index]);
        }
        // replace the data with the last
        if (index < hp->len - 1){
            hp->data[index] = hp->data[hp->len - 1];
        }
        hp->data[hp->len - 1] = NULL;
        hp->len -= 1;
        // adjust the heap
        if (hp->len > 1 && index < hp->len){
            if (index > 0 && index >= (hp->len >> 1)){
                index = (index - 1) >> 1;
            }
            while (index > 0){
                HEAP_ADJUST(hp, index);
                index = (index - 1) >> 1;
            }
            HEAP_ADJUST(hp, 0);
        }
    }
}

/* ------------------------------------- 
 * pop the heap top element 
 * do not free the data memory space
 * ------------------------------------- */
void * heap_pop(Heap * hp){
    void * top = hp->data[0];
    hp->data[0] = hp->data[hp->len - 1];
    hp->data[hp->len - 1] = NULL;
    hp->len -= 1;
    if (hp->len > 1)
        HEAP_ADJUST(hp,0);
    return top;
}
/* ----------------------------------
 * Free the Heap Memory Space
 * If given the item free function
 * free the item data memory space 
 * ---------------------------------- */
void   heap_free(Heap * hp){
    if (hp->data){
        for (int i = 0; i < hp->len; i++){
            if (NULL != hp->heap_item_free_fn){
                hp->heap_item_free_fn(hp->data[i]);
            }else{
                free(hp->data[i]);
            }
            hp->data[i] = NULL;
        }
        free(hp->data);
        hp->data = NULL;
    }
    free(hp);
}
