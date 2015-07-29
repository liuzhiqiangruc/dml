/* ========================================================
 *   Copyright (C) 2014 All rights reserved.
 *   
 *   filename : mtrace.h
 *   author   : liuzhiqiang01@baidu.com
 *   date     : 2014-12-31
 *   info     : median value trace based on rb tree
 * ======================================================== */

#ifndef _MTRACE_H
#define _MTRACE_H

#include "fn_type.h"
#include "rbtree.h"

/* -----------------------------------
 * Median Trace struct based on avl
 * av1 : left half tree
 * hp2 : right half tree
 * ----------------------------------- */

typedef struct _median_t{
    RBTree * rb1;    /* left tree */
    RBTree * rb2;    /* right tree */
    void * left_max; /* max element in left tree */
    void * right_min;/* min element in right tree */
    int cnt_1;       /* element cnt in left tree */
    int cnt_2;       /* element cnt in right tree */
    CMP_FN cmp_fn;   /* compare function */
    FREE_FN free_fn; /* free function */
} MTrace;



/* -----------------------------------
 * Interface for operation
 * ----------------------------------- */
MTrace * m_create(CMP_FN cmp_fn, FREE_FN free_fn);
void     m_add(MTrace * m, void *pdata);
void     m_remove(MTrace * m, void * pdata);
void   * get_median(MTrace * m);
void     m_free(MTrace * m);
void     m_clear(MTrace * m);


#endif //MTRACE_H

