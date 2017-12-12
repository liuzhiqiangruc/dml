/* ========================================================
 *   Copyright (C) 2015 All rights reserved.
 *   
 *   filename : auc.c
 *   author   : ***
 *   date     : 2015-08-19
 *   info     : auc calculation
 * ======================================================== */

#include <stdio.h>
#include <stdlib.h>
#include "auc.h"

typedef struct _aucP {
    double x;
    int id;
}AucP;

int cmp(const void *_a, const void *_b) {
    AucP *a = (AucP *) _a;
    AucP *b = (AucP *) _b;
    double l = a->x - b->x;
    return l > 0 ? 1 : (l < 0 ? -1 : 0);
}

void tiedrank(int n, AucP *aucp, double *rk) {
    double curval;
    int i, j, lastpos;
    qsort(aucp, n, sizeof(aucp[0]), cmp);
    curval = aucp[0].x, lastpos = 0;
    for (i = 0; i < n; ++i) {
        if (curval > aucp[i].x + 1e-9 || curval < aucp[i].x - 1e-9) {
            for (j = lastpos; j < i; ++j) {
                rk[aucp[j].id] = (double)(lastpos + i + 1) / 2.;
            }
            lastpos = i;
            curval = aucp[i].x;
        }
    }
    for (j = lastpos; j < i; ++j) {
        rk[aucp[j].id] = (double)(lastpos + i + 1) / 2.;
    }
}

double auc(int n, double *x, double *y) {
    if (!y || !x) return 0.0;
    double *rk = (double*) malloc(sizeof(double) * n);
    AucP *aucp = (AucP *)malloc(sizeof(AucP) * n);
    int i, tsum;
    double rksum, auc;
    for (i = 0; i < n; ++i) {
        aucp[i].x = x[i];
        aucp[i].id = i;
    }
    tiedrank(n, aucp, rk);
    for (rksum = 0., tsum = 0, i = 0; i < n; ++i) {
        if (y[i] >= 1. - 1e-10) {
            rksum += rk[i];
            tsum += 1;
        }
    }
    double mn, pst;
    mn = (double) (n - tsum);
    mn *= (double) tsum;
    pst = (double) tsum;
    pst *= (double) tsum + 1;
    auc = (rksum - pst / 2.) / mn;
    free(rk);
    free(aucp);
    return auc;
}

