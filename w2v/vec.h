/* ========================================================
 *   Copyright (C) 2016 All rights reserved.
 *   
 *   filename : vec.h
 *   author   : liuzhiqiangruc@126.com
 *   date     : 2016-10-27
 *   info     : 
 * ======================================================== */

#ifndef _VEC_H
#define _VEC_H

#include "tsdata.h"

typedef struct _vec {
    int v;            // voc size
    int k;            // length of vector
    int t;            // model flag, train, continue, infer
    int (*hbt)[5];    // huffman tree {p0, b0, p1, b1, indx}
    float  * neu0;    // doc|word vec representation
    float  * neu1;    // softmax tree vec representation
} Vec;

void vec_build_tree(Vec * vec, int (*wc)[2], int n);
void vec_learn_tree(Vec * vec, float * cw, float * de, int vid, double learn_rate);
void vec_save_tree (Vec * vec, TSD * ds, char * outdir);
void vec_load_tree (Vec * vec, TSD * ds, char * outdir, char * leaff);
void vec_free_tree (Vec * vec);

#endif //VEC_H
