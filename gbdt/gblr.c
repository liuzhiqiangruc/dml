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
    double train_auc = auc(y_rowns(g), y_model(g), y_label(g));
    printf("current tree size : %4d, train_auc : %.3f", t_size(g), train_auc);
    if (has_test(g) == 1){
        double test_auc  = auc(t_rowns(g), t_model(g), t_label(g));
        printf(" test_auc : %.3f", test_auc);
    }
    printf("\n");
}


GBDT * gbdt_lr(GBMP p){
    return gbdt_create(lr_grad, lr_hess, lr_repo, p);
}

