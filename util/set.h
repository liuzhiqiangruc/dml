/* ========================================================
 *   Copyright (C) 2015 All rights reserved.
 *   
 *   filename : set.h
 *   author   : liuzhiqiang01@baidu.com
 *              lizeming@baidu.com
 *   date     : 2015-03-20
 *   info     : set define using rb tree
 * ======================================================== */

#ifndef _SET_H
#define _SET_H

#include "fn_type.h"
#include "rbtree.h"

#define set_value(st) ((st)->pCurrent->pData)

/* ------------------------------
 * Set Struct Define
 * ------------------------------ */
typedef struct _set {
    int size;
    RBTree * rb;
    RBNode * pCurrent;
} Set;

/* --------------------------------------
 * Set Operation Define
 * -------------------------------------- */
Set * set_create(CMP_FN cmp_fn, FREE_FN free_fn);
int   set_add(Set * st, void * pdata);
int   set_del(Set * st, void * pdata);
int   set_in (Set * st, void * pdata);
void  set_free(Set * st);
void  set_next(Set * st);
void  reset(Set *st);
int   isEnd(Set *st);

#endif //SET_H

