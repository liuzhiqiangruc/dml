/* ========================================================
 *   Copyright (C) 2016 All rights reserved.
 *   
 *   filename : gblr.c
 *   author   : liuzhiqiangruc@126.com
 *   date     : 2016-02-26
 *   info     : implementation for gblr
 * ======================================================== */
#include <stdio.h>
#include <math.h>
#include "auc.h"
#include "gblr.h"


void lr_grad(double *f, double *y, double *g, int n){
    int i;
    double sig;
    for (i = 0; i < n; i++){
        if (f[i] > 12.0){
            sig = 0.99999;
        }
        else if (f[i] < -12.0){
            sig = 0.00001;
        }
        else{
            sig = 1.0 / (1.0 + exp(-f[i]));
        }
        g[i] = sig - y[i];
    }
}

void lr_hess(double *f, double *y, double *h, int n){
    int i;
    double sig;
    for (i = 0; i < n; i++){
        if (f[i] > 12.0){
            sig = 0.99999;
        }
        else if (f[i] < -12.0){
            sig = 0.00001;
        }
        else{
            sig = 1.0 / (1.0 + exp(-f[i]));
        }
        h[i] = sig * (1.0 - sig);
    }
}

void lr_repo(GBDT * g){
    double train_auc = auc(g->train_ds->row, g->f, g->train_ds->y);
    double test_auc  = auc(g->test_ds->row, g->t, g->test_ds->y);
    printf("current tree size : %4d, train_auc : %.3f, test_auc : %.3f \n", g->tree_size, train_auc, test_auc);
}


GBDT * gbdt_lr(GBMP p){
    return gbdt_create(lr_grad, lr_hess, lr_repo, p);
}

