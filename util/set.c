/* ========================================================
 *   Copyright (C) 2015 All rights reserved.
 *   
 *   filename : set.c
 *   author   : liuzhiqiang01@baidu.com
 *              lizeming@baidu.com
 *   date     : 2015-03-20
 *   info     : implementation of set
 * ======================================================== */

#include <stdio.h>
#include <stdlib.h>
#include "set.h"

Set *set_create(CMP_FN cmp_fn, FREE_FN free_fn) {
    Set *t = (Set *) malloc(sizeof(Set));
    t->rb = rb_create(cmp_fn, free_fn);
    t->pCurrent = t->rb->nil;
    t->size = 0;
    return t;
}

/* ---------------------------------
 * brief  : check if pdata is in set
 * return : find 1, not find 0
 * --------------------------------- */
int set_in(Set *st, void *pdata) {
    if (node_find(st->rb, pdata) != NULL) return 1;
    return 0;
}

/* -----------------------------------
 * brief  : add pdata to set
 * return : success 0, failed -1
 * ----------------------------------- */
int set_add(Set *st, void *pdata) {
    if (set_in(st, pdata) != 1){
        if (rb_insert(st->rb, pdata) == -1) return -1;
        st->size += 1;
    }
    return 0;
}

/* ---------------------------------
 * brief : delete pdata to set
 * return : success 0, failed -1
 * --------------------------------- */
int set_del(Set *st, void *pdata) {
    if (rb_delete(st->rb, pdata) != -1){
        st->size -= 1;
    }
    return 0;
}

void set_next(Set *st) {
    RBNode *nil = st->rb->nil;
    if (st->pCurrent->child[1] != nil) {
        st->pCurrent = st->pCurrent->child[1];
        while (st->pCurrent->child[0] != nil) {
            st->pCurrent = st->pCurrent->child[0];
        }
    } else {
        while (st->pCurrent->p != nil && st->pCurrent->p->child[1] == st->pCurrent) {
            st->pCurrent = st->pCurrent->p;
        }
        st->pCurrent = st->pCurrent->p;
    }
}

void set_free(Set *st) {
    rb_free(st->rb);
    free(st);
}

void reset(Set *st) {
    if (st->size == 0){
        st->pCurrent = st->rb->nil;
        return ;
    }
    st->pCurrent = st->rb->root;
    while(st->pCurrent->child[0] != st->rb->nil) {
        st->pCurrent = st->pCurrent->child[0];
    }
}

int isEnd(Set *st) {
    return (st->pCurrent == st->rb->nil);
}
