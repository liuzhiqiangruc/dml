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
    double rate;                 /* learning rate      */
    double nod_reg;              /* regular for node   */
    double wei_reg;              /* regular for weight */
    int    max_leaf_nodes;       /* max leaf nodes     */
    int    max_trees;            /* tree capacity      */
    int    max_depth;            /* max tree depth     */
    int    min_node_ins;         /* min ins. for node  */
    int    binary;               /* 1 for binary       */
    char * train_init;           /* train init model   */
    char * train_input;          /* train input file   */
    char * test_init;            /* test  init model   */
    char * test_input;           /* test  input file   */
    char * out_dir;              /* output dir         */
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
