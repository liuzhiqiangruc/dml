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

// default do nothing
void mlr_hess(double *f, double *y, double *h, int n, int k){}

void mlr_grad(double *f, double *y, double *e, int n, int k){
    double * t = (double *)calloc(n, sizeof(double));
    int i, j, offs;
    for (i = 0; i < k; i++){
        offs = i * n;
        for (j = 0; j < n; j++){
            e[offs + j] = exp(f[offs + j]);
            t[j]       += e[offs + j];
        }
    }
    for (i = 0; i < k; i++){
        offs = i * n;
        for (j = 0; j < n; j++){
            e[offs + j] /= t[j];
            if (y[j] == i){
                e[offs + j] = 1.0 - e[offs + j];
            }
            else{
                e[offs + j] = -e[offs + j];
            }
        }
    }
    free(t); t = NULL;
    return;
}


void mlr_repo(GBM *gbm){
    // just print train score for debug
    //
    //
    int i, j, k, n, offs;
    double *f = NULL;
    n = y_rowns(gbm);
    k = k_count(gbm);
    f = y_model(gbm);
    for (i = 0; i < n; i++){
        for(j = 0; j < k; j++){
            offs = j * n + i;
            fprintf(stderr, "\t%.5f", f[offs]);
        }
        fprintf(stderr, "\n");
    }
    fprintf(stderr, "========\n");
}

GBM * gbm_lr(GBMP p){
    return gbm_create(mlr_grad, mlr_hess, mlr_repo, p);
}
