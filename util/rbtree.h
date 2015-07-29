/* ========================================================
 *   Copyright (C) 2014 All rights reserved.
 *
 *   filename : rbtree.h
 *   author   : liuzhiqiang01@baidu.com
 *              lizeming@baidu.com
 *   date     : 2014-12-15
 *   info     : Red Black Tree Interface
 * ======================================================== */

#ifndef _RBT_H
#define _RBT_H

#include "fn_type.h"

#define RB_RED   0
#define RB_BLACK 1

/* ----------------------------
 * Node and Tree define
 * ---------------------------- */
typedef struct _RBNode{
    void *pData;
    int   color;
    struct _RBNode *child[2];
    struct _RBNode * p;
} RBNode;

typedef struct _RBTree{
    RBNode * root;
    RBNode * nil;
    CMP_FN   cmp_fn;
    FREE_FN  free_fn;
}RBTree;

/* ------------------------------------
 * Functions
 * ------------------------------------ */
RBTree * rb_create(CMP_FN cmp_fn, FREE_FN fn);
int rb_insert(RBTree *t, void *pdata);
int rb_delete(RBTree *t, void *pdata);
RBNode * node_find(RBTree *t, void *pdata);
void * rb_find_left(RBTree *t);
void * rb_find_right(RBTree *t);
void rb_free(RBTree *t);
void rb_clear(RBTree *t);

#endif //RBT_H

