/* ========================================================
 *   Copyright (C) 2017 All rights reserved.
 *   
 *   filename : gbrank.c
 *   author   : ***
 *   date     : 2017-09-05
 *   info     : implementation for gbrank
 * ======================================================== */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include "auc.h"
#include "gbrank.h"

// rand int between [s, e)
int rand_range(int s, int e){
    int sp = e - s;
    int d = (0.1 + rand()) / (RAND_MAX + 1.0) * sp;
    return d + s;
}

void rank_grad(double *f, double *y, double *g, int n, GBMP * gbmp){
    int i, j, k, p, s, t;
    int *idx = (int*)calloc(n, sizeof(int));
    double *sig = (double*)calloc(n, sizeof(double));
    double M = gbmp->max_margin;
    p = 0;
    j = 0;
    k = n - 1;
    s = 5;
    for (i = 0; i < n; i++){
        sig[i] = 1.0 / (1 + exp(-f[i]));
        if (1.0 == y[i]){
            p += 1;
            idx[j++] = i;
        }
        else {
            idx[k--] = i;
        }
    }
    srand(time(NULL));
    memset(g, 0, sizeof(double) * n);
    for (i = 0; i < n; i++){
        j = idx[i];
        if (1.0 == y[j]){
            k = rand_range(p, n - s + 1);
            for (t = k; t < k + s; t++) if (sig[j] - sig[idx[t]] < M){
                g[j] += (-M + sig[j] - sig[idx[t]]) * sig[j] * (1 - sig[j]);
            }
        }
        else{
            k = rand_range(0, p - s + 1);
            for (t = k; t < k + s; t++) if (sig[idx[t]] - sig[j] < M){
                g[j] += (M + sig[j] - sig[idx[t]]) * sig[j] * (1 - sig[j]);
            }
        }
    }
    free(idx); idx = NULL;
}

void rank_hess(double *f, double *y, double *h, int n, GBMP * p){
    int i;
    for (i = 0; i < n; i++){
        h[i] = 1.0;
    }
}

void rank_repo(GBDT * g){
    double train_auc = auc(y_rowns(g), y_model(g), y_label(g));
    fprintf(stderr, "current tree size : %4d, train_auc : %.3f", t_size(g), train_auc);
    if (has_test(g) == 1){
        double test_auc  = auc(t_rowns(g), t_model(g), t_label(g));
        fprintf(stderr, " test_auc : %.3f", test_auc);
    }
    fprintf(stderr, "\n");
}


GBDT * gbdt_rank(GBMP p){
    return gbdt_create(rank_grad, rank_hess, rank_repo, p);
}
