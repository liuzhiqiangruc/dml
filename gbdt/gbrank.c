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

void rank_grad_bak(double *f, double *y, double *g, int n, GBMP * gbmp){
    int i, j, k, p, s, t;
    int *idx = (int*)calloc(n, sizeof(int));
    double M = gbmp->max_margin;
    p = 0;
    j = 0;
    k = n - 1;
    s = 10;
    for (i = 0; i < n; i++){
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
            for (t = k; t < k + s; t++) if (f[j] - f[idx[t]] < M){
                g[j] += -M + f[j] - f[idx[t]];
                g[idx[t]] += M + f[idx[t]] - f[j];
            }
        }
        else{
            k = rand_range(0, p - s + 1);
            for (t = k; t < k + s; t++) if (f[idx[t]] - f[j] < M){
                g[j] +=  M + f[j] - f[idx[t]];
                g[idx[t]] += -M + f[idx[t]] - f[j];
            }
        }
    }
    free(idx); idx = NULL;
}

void rank_grad(double *f, double *y, double *g, int n, GBMP * gbmp){
    int i, j, k, p, s, t;
    int *idx = (int*)calloc(n, sizeof(int));
    double M = gbmp->max_margin;
    p = 0;
    j = 0;
    k = n - 1;
    s = 10;
    for (i = 0; i < n; i++){
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
            for (t = k; t < k + s; t++) if (f[j] - f[idx[t]] < M){
                g[j] += -M + f[j] - f[idx[t]];
            }
        }
        else{
            k = rand_range(0, p - s + 1);
            for (t = k; t < k + s; t++) if (f[idx[t]] - f[j] < M){
                g[j] +=  M + f[j] - f[idx[t]];
            }
        }
    }
    free(idx); idx = NULL;
}

// multi class ranking gradient function
void rank_grad_m(double *f, double *y, double *g, int n, GBMP * gbmp){
    int nc[1001] = {0}; // most 1000 class model
    int i, j, k, c, s, t, r;
    int *idx = (int*)calloc(n, sizeof(int));
    double M = gbmp->max_margin;
    s = 4;   // other class instance sample count
    for (i = 0; i < n; i++){
        j = (int)y[i];
        if (j < 1000) nc[j + 1] += 1;
    }
    i = 0;
    while (nc[++i] > 0){
        nc[i] += nc[i - 1];
    }
    k = i - 1;   // class count for current dataset
    for (i = 0; i < n; i++){
        j = (int)y[i];
        idx[nc[j]++] = i;
    }
    srand(time(NULL));
    memset(g, 0, sizeof(double) * n);
    for (i = 0; i < n; i++){
        j = idx[i];
        for (c = 0; c < k; c++){
            r = rand_range(c == 0 ? 0 : nc[c - 1], nc[c] - s + 1);
            for (t = r; t < r + s; t++){
                if (c < (int) y[j]){
                    if (f[j] - f[idx[t]] < M){
                        g[j] += -M + f[j] - f[idx[t]];
                        g[idx[t]] +=  M + f[idx[t]] - f[j];
                    }
                }
                else if (c > (int)y[j]){
                    if (f[idx[t]] - f[j] < M){
                        g[j] += M + f[j] - f[idx[t]];
                        g[idx[t]] += -M + f[idx[t]] - f[j];
                    }
                }
            }
        }
    }
    free(idx); idx = NULL;
    return;
}

void rank_hess(double *f, double *y, double *h, int n, GBMP * p){
    int i;
    for (i = 0; i < n; i++){
        h[i] = 1.0;
    }
}

void rank_repo(GBDT * g){
    double train_auc = auc(y_rowns(g), y_model(g), y_label(g));
    fprintf(stderr, "current tree size : %4d, train_auc : %.8f", t_size(g), train_auc);
    if (has_test(g) == 1){
        double test_auc  = auc(t_rowns(g), t_model(g), t_label(g));
        fprintf(stderr, " test_auc : %.8f", test_auc);
    }
    fprintf(stderr, "\n");
}

GBDT * gbdt_rank(GBMP p){
    return gbdt_create(rank_grad_m, rank_hess, rank_repo, p);
}
