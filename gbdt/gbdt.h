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
#include "fn_types.h"
#include "gbcfg.h"


//- type declare
typedef struct _gbdt GBDT;

//- report function define
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
