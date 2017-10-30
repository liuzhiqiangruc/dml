/* ========================================================
 *   Copyright (C) 2017 All rights reserved.
 *   
 *   filename : gbmr.c
 *   author   : ***
 *   date     : 2017-07-20
 *   info     : 
 * ======================================================== */
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include "auc.h"
#include "gbmr.h"

static void exp_norm(double *f, double *e, int n, int k){
    double *t = (double *)calloc(n, sizeof(double));
    int i, j, offs;
    for(i = 0; i < k; i++){
        offs = i * n;
        for (j = 0; j < n; j++){
            e[offs + j] = exp(f[offs + j]);
            t[j] += e[offs + j];
        }
    }
    for (i = 0; i < k; i++){
        offs = i * n;
        for (j = 0; j < n; j++){
            e[offs + j] /= t[j];
        }
    }
    free(t); t = NULL;
    return;
}

void mlr_grad(double *f, double *y, double *e, double *h, int n, int k){
    int i, j, offs;
    double s;
    exp_norm(f, e, n, k);
    for (i = 0; i < n * k; i++){
        h[i] = e[i] * (1.0 - e[i]);
    }
    for (i = 0; i < k; i++){
        offs = i * n;
        for (j = 0; j < n; j++){
            s = (y[j] == i ? 1.0 : 0.0);
            e[offs + j] -= s;
        }
    }
    return;
}

static double k_loss(double * e, double * y, int n, int k){
    int i, j;
    double s = 0.0;
    for (i = 0; i < n; i++){
        for (j = 0; j < k; j++){
            if (y[i] == j){
                s -= log(e[j * n + i]);
            }
        }
    }
    return s;
}

void mlr_repo(GBM *gbm){
    int k, n, c;
    double train_loss, test_loss, train_auc, test_auc;
    double *f, *e, *y;
    c = t_size(gbm);
    n = y_rowns(gbm);
    k = k_count(gbm);
    f = y_model(gbm);
    y = y_label(gbm);
    e = (double*)calloc(n * k, sizeof(double));
    exp_norm(f, e, n, k);
    train_loss = k_loss(e, y, n , k);
    train_auc  = 1.0 - auc(n, e, y);
    fprintf(stderr, "current tree size: %4d, train loss: %10.5f, train auc : %.8f", c, train_loss, train_auc);
    if (1 == has_test(gbm)){
        n = t_rowns(gbm);
        f = t_model(gbm);
        y = t_label(gbm);
        free(e);
        e = (double*)calloc(n * k, sizeof(double));
        exp_norm(f, e, n , k);
        test_loss = k_loss(e, y, n, k);
        test_auc  = 1.0 - auc(n, e, y);
        fprintf(stderr, ", test loss: %10.5f, test auc: %.8f", test_loss, test_auc);
    }
    free(e); e = NULL;
    fprintf(stderr, "\n");
    return;
}

GBM * gbm_lr(GBMP p){
    return gbm_create(mlr_grad, mlr_repo, p);
}
