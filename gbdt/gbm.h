/* ========================================================
 *   Copyright (C) 2017 All rights reserved.
 *   
 *   filename : gbm.h
 *   author   : ***
 *   date     : 2017-07-19
 *   info     : gbdt for mutil normial
 * ======================================================== */

#ifndef _GBM_H
#define _GBM_H

#include "data.h"
#include "dtree.h"
#include "gbcfg.h"


//- type declare
typedef struct _gbm GBM;

/* --------------------------------------
 * brief  : f', f" and report functions
 * f      : current model value
 * y      : target label
 * g      : f' outer
 * h      : f" outer
 * n      : length of f,y,g,h
 * k      : total class count 
 * -------------------------------------- */
typedef void(*FN_GH)(double * f, double * y, double * g, double * h, int n, int k);

//- report function define
typedef void(*FN_R)(GBM * m);

//- class count k
int      k_count(GBM * gbm);

//-   train rows, labels, and model
int      y_rowns(GBM * gbm);
int      y_colns(GBM * gbm);
double * y_label(GBM * gbm);
double * y_model(GBM * gbm);

//-   test  rows, labels, and model
int      t_rowns(GBM * gbm);
int      t_colns(GBM * gbm);
double * t_label(GBM * gbm);
double * t_model(GBM * gbm);

//-   tree size currently
int      t_size(GBM * gbm);

//-   has test data or not
int      has_test(GBM * gbm);

//-   create, train, save, free gbm model
GBM * gbm_create(FN_GH g_fn, FN_R r_fn, GBMP p);
int   gbm_train(GBM * gbm);
void  gbm_save (GBM * gbm);
void  gbm_free (GBM * gbm);

#endif //GBM_H
