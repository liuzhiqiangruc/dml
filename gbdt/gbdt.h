/* ========================================================
 *   Copyright (C) 2016 All rights reserved.
 *   
 *   filename : gbdt.h
 *   author   : liuzhiqiangruc@126.com
 *   date     : 2016-02-26
 *   info     : gradient boosting decision tree model
 * ======================================================== */

#ifndef _GBDT_H
#define _GBDT_H

#include "dtree.h"
#include "tdata.h"

typedef struct _gbdt GBDT;

/* --------------------------------------
 * brief  : f' and f" function definition
 * f      : current model value
 * y      : target label
 * g      : f' outer
 * h      : f" outer
 * n      : length of f,y,g,h
 * -------------------------------------- */
typedef void(*G)(double * f, double * y, double * g, int n);
typedef void(*H)(double * f, double * y, double * h, int n);
typedef void(*R)(GBDT * m);

typedef struct _gbdt_param {
    double rate;
    int max_depth;
    int max_trees;
    char * train_input;
    char * test_input;
    char * out_dir;
} GBMP;

struct _gbdt {
    DTD * train_ds;
    DTD *  test_ds;
    double * f;
    double * t;
    int tree_size;
    DTree ** dts;
    GBMP p;
    G g_fn;
    H h_fn;
    R r_fn;
};

GBDT * gbdt_create(G g_fn, H h_fn, R f_fn, GBMP p);
int    gbdt_train(GBDT * gbdt);
void   gbdt_save (GBDT * gbdt, int n);
void   gbdt_free (GBDT * gbdt);

#endif //GBDT_H

