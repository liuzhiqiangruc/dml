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

/* --------------------------------------
 * GBDT configurations 
 * -------------------------------------- */
typedef struct _gbdt_param {
    double rate;
    int max_leaf_nodes;
    int max_trees;
    int binary;
    char * train_input;
    char * test_input;
    char * out_dir;
} GBMP;

typedef struct _gbdt GBDT;
/* --------------------------------------
 * brief  : f', f" and report functions
 * f      : current model value
 * y      : target label
 * g      : f' outer
 * h      : f" outer
 * n      : length of f,y,g,h
 * -------------------------------------- */
typedef void(*G)(double * f, double * y, double * g, int n);
typedef void(*H)(double * f, double * y, double * h, int n);
typedef void(*R)(GBDT * m);

//-   train rows, labels, and model
int      y_rowns(GBDT * gbdt);
double * y_label(GBDT * gbdt);
double * y_model(GBDT * gbdt);

//-   test  rows, labels, and model
int      t_rowns(GBDT * gbdt);
double * t_label(GBDT * gbdt);
double * t_model(GBDT * gbdt);

//-   tree size currently
int      t_size(GBDT * gbdt);

//-   has test data or not
int      has_test(GBDT * gbdt);

//-   create, train, save, free gbdt model
GBDT * gbdt_create(G g_fn, H h_fn, R f_fn, GBMP p);
int    gbdt_train(GBDT * gbdt);
void   gbdt_save (GBDT * gbdt);
void   gbdt_free (GBDT * gbdt);

#endif //GBDT_H
