#include "lr.h"
#include "fn_type.h"
#include <string.h>
#include "newton_opt.h"
#include <stdlib.h>
#include <math.h>


typedef struct _dataset {
    int r;        // r instance
    int c;        // c coefficient
//    int tlen;     // number of data
    double *val; //  <id , val> :  <key, value>
    int *id;
    int *len;     //length of each instance
    double *y;    //label of each instance
} Dataset;

void dataset_free(Dataset *ds) {
    if (ds->val != NULL) {
        free(ds->val);
    }
    if (ds->id != NULL) {
        free(ds->id);
    }
    if (ds->len != NULL) {
        free(ds->len);
    }
    if (ds->y != NULL) {
        free(ds->y);
    }
    free(ds);
}

double lr_eval(double *x, void *_ds) {
    int offs, i, j;
    double loss = 1;
    double yest;
    double add;
    Dataset *ds = (Dataset *) _ds;
    double *val = ds->val;
    int *id = ds->id;
    int *len = ds->len;

    for (offs = i = 0; i < ds->r; offs += len[i], ++i) {
        for (yest = 0., j = 0; j < len[i]; ++j) {
            yest += val[offs + j] * x[id[offs + j]];
        }
        if (ds->y[i] < 0.5) {
            yest = -yest;
        }
        if (yest < -30) {
            add = -yest;
        } else if (yest > 30) {
            add = 0;
        } else {
            add = log(1 + exp(-yest));
        }
        loss += add;
    }
    return loss;
}

void lr_grad(double *x, void *_ds, double *g) {
    int i, j, offs;
    double yest;
    double hx;
    Dataset *ds = (Dataset *) _ds;
    double *val = ds->val;
    double *y = ds->y;
    int *id = ds->id;
    int *len = ds->len;
    memset(g, 0, sizeof(double) * ds->c);
    for (offs = i = 0; i < ds->r; offs += len[i], ++i) {
        for (yest = 0., j = 0; j < len[i]; ++j) {
            yest += val[offs + j] * x[id[offs + j]];
        }
        if (yest < -30) {
            hx = 0.;
        } else if (yest > 30) {
            hx = 1.;
        } else {
            hx = 1. / (1 + exp(-yest));
        }
        for (j = 0; j < len[i]; ++j) {
            g[id[offs + j]] += (hx - y[i]) * val[offs + j];
        }
    }
}

/*
 * r:    r instance
 * c:    c coefficient
 * tlen: number of data
 * len:  length of each instance
 * id:
 * val:  <id , val> :  <key, value>
 * y:    label of each instance
 * x:    return coefficient
 */
int lr(int r, int c, int tlen, int *len, int *id, double *val, double *y, double *x) {
    Dataset *ds = (Dataset *) malloc(sizeof(Dataset));
    ds->val = (double *) malloc(sizeof(double) * tlen);
    ds->id = (int *) malloc(sizeof(int) * tlen);
    ds->y = (double *) malloc(sizeof(double) * r);
    ds->len = (int *) malloc(sizeof(int) * r);
    ds->r = r;
    ds->c = c;
    memcpy(ds->val, val, sizeof(double) * tlen);
    memcpy(ds->id, id, sizeof(int) * tlen);
    memcpy(ds->y, y, sizeof(double) * r);
    memcpy(ds->len, len, sizeof(int) * r);
    lbfgs(ds, lr_eval, lr_grad, 1e-9, 5, c, 1000, x);
    dataset_free(ds);
    return 0;
}