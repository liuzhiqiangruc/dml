/* ========================================================
 *   Copyright (C) 2013 All rights reserved.
 *   
 *   filename : heap.h
 *   author   : liuzhiqiang01@baidu.com
 *   date     : 2013-11-17
 *   info     : heap struct implementation
 * ======================================================== */

#ifndef HEAP_H
#define HEAP_H

/* ---------------------------------
 * Data Define for Heap Structor
 * --------------------------------- */
typedef int  (*HEAP_CMP_FN) (void *, void*);
typedef void (*HEAP_ITEM_FREE)(void *);

typedef struct _heap {
    int len;
    int size;
    void ** data;
    HEAP_CMP_FN heap_cmp_fn;
    HEAP_ITEM_FREE heap_item_free_fn;
} Heap;


/* ---------------------------------------------
 * Interface Function Define for Heap Operation
 * --------------------------------------------- */
Heap * heap_create(int size);
void   heap_add(Heap * hp, void * pdata);
void   heap_remove(Heap * hp, void * pdata);
void * heap_pop(Heap * hp);
void   heap_free(Heap * hp);

#endif
