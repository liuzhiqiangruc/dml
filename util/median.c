/* ========================================================
 *   Copyright (C) 2016 All rights reserved.
 *   
 *   filename : median.c
 *   author   : ***
 *   date     : 2016-01-15
 *   info     : 
 * ======================================================== */

#include <stdlib.h>
#include "median.h"

int mt_cmp(int * m, int *n){
    return *m > *n ? 1 : (*m < *n ? -1 : 0);
}

static void m_insert_left(MTrace * m, void *pdata){
    rb_insert(m->rb1, pdata);
    m->cnt1 += 1;
    if ((m->left_max && m->cmp_fn(pdata, m->left_max) > 0) || (!m->left_max)){
        m->left_max = pdata;
    }
}

static void m_insert_right(MTrace * m, void * pdata){
    rb_insert(m->rb2, pdata);
    m->cnt2 += 1;
    if ((m->right_min && m->cmp_fn(pdata, m->right_min) < 0) || (!m->right_min)){
        m->right_min = pdata;
    }
}

static void * find_left_max(MTrace * m){
    return rb_max_value(m->rb1);
}

static void * find_right_min(MTrace * m){
    return rb_min_value(m->rb2);
}

static void balance(MTrace * m){
    if (m->cnt1 > m->cnt2 + 1){
        m_insert_right(m, m->left_max);
        rb_delete(m->rb1, m->left_max);
        m->cnt1 -= 1;
        m->left_max = find_left_max(m);
    }
    else if (m->cnt1 + 1 < m->cnt2){
        m_insert_left(m, m->right_min);
        rb_delete(m->rb2, m->right_min);
        m->cnt2 -= 1;
        m->right_min = find_right_min(m);
    }
}

MTrace * m_create(CMP_FN cmp_fn, FREE_FN free_fn){
    MTrace * mt = (MTrace*)malloc(sizeof(MTrace));
    if (!mt){
        return NULL;
    }
    // functions
    mt->cmp_fn     = (CMP_FN) mt_cmp;
    if (cmp_fn)
        mt->cmp_fn     = cmp_fn;
    mt->free_fn    = free_fn;
    
    mt->rb1 = rb_create(mt->cmp_fn, mt->free_fn);
    mt->rb2 = rb_create(mt->cmp_fn, mt->free_fn);
    
    mt->left_max   = NULL;
    mt->right_min  = NULL;
    mt->cnt1       = 0;
    mt->cnt2       = 0;

    // return 
    return mt;
}

void m_add(MTrace * m, void *pdata){
    if (m->cnt1 == 0 || m->cmp_fn(pdata, m->left_max) > 0){
        m_insert_right(m, pdata);
    }
    else{
        m_insert_left(m, pdata);
    }
    balance(m);
}

void m_remove(MTrace * m, void * pdata){
    if (m->cmp_fn(pdata, m->left_max) > 0){
        rb_delete(m->rb2, pdata);
        m->cnt2 -= 1;
        m->right_min = find_right_min(m);
    }
    else{
        rb_delete(m->rb1, pdata);
        m->cnt1 -= 1;
        m->left_max = find_left_max(m);
    }
    balance(m);
}

void * get_median(MTrace * m){
    if (m->cnt1 > 0 || m->cnt2 > 0){
        if (m->cnt1 >= m->cnt2){
            return m->left_max;
        }
        else{
            return m->right_min;
        }
    }
    return NULL;
}

void m_free(MTrace * m){
    if (m){
        if (m->rb1){
            rb_free(m->rb1);
            m->rb1 = NULL;
        }
        if (m->rb2){
            rb_free(m->rb2);
            m->rb2 = NULL;
        }
        free(m);
    }
}

void m_clear(MTrace * m){
    if (m->rb1){
        rb_clear(m->rb1);
    }
    if (m->rb2){
        rb_clear(m->rb2);
    }
    m->left_max  = NULL;
    m->right_min = NULL;
    m->cnt1 = m->cnt2 = 0;
}

