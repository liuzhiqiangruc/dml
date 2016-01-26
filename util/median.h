/* ========================================================
 *   Copyright (C) 2016 All rights reserved.
 *   
 *   filename : median.h
 *   author   : ***
 *   date     : 2016-01-15
 *   info     : 
 * ======================================================== */

#ifndef _MEDIAN_H
#define _MEDIAN_H

#include "rbtree.h"

typedef struct _median {
    RBTree * rb1;
    RBTree * rb2;
    void * left_max;
    void * right_min;
    int cnt1;
    int cnt2;
    CMP_FN cmp_fn;
    FREE_FN free_fn;
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




#endif //MEDIAN_H
