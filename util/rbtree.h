/* ========================================================
 *   Copyright (C) 2015 All rights reserved.
 *   
 *   filename : rbtree.h
 *   author   : ***
 *   date     : 2015-12-29
 *   info     : 
 * ======================================================== */

#ifndef _RBTREE_H
#define _RBTREE_H

typedef int    (*CMP_FN)(void *, void*);
typedef void   (*FREE_FN)(void *);

// RBTree declaration
typedef struct _RBTree RBTree;

// create and return a rb tree
// by given comapre function cmp_fn and 
// free function free_fn
RBTree * rb_create(CMP_FN cmp_fn, FREE_FN free_fn);

// insert data into rbtree t
// return the corelated index in tree
// return -1 if failed
int  rb_insert(RBTree * t, void *data);

// delete data from rbtree t
// return the old corelated index in tree
// return -1 if not found data in tree or failed
int  rb_delete(RBTree * t, void *data);

// fetch the correlated index of data in tree t
// return -1 if not found
int  rb_index (RBTree * t, void *data);

// clear the rbtree and keep the tree space
// for future usage
void rb_clear (RBTree * t);

// free the rbtree space and destory the rbtree
void rb_free  (RBTree * t);

// find max element pointer in rbtree
void * rb_max_value(RBTree * t);

// find min element pointer in rbtree
void * rb_min_value(RBTree * t);

#endif //RBTREE_H
