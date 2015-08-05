/* ========================================================
 *   Copyright (C) 2015 All rights reserved.
 *   
 *   filename : lda.c
 *   author   : liuzhiqiang01@baidu.com
 *              lizeming@baidu.com
 *   date     : 2015-03-26
 *   info     : implementation for LR with L1, L2 Norm
 *              using newton method : owlqn, lbfgs
 * ======================================================== */

#include <math.h>
#include <stdlib.h>
#include <string.h>

#include "fn_type.h"
#include "newton_opt.h"
#include "lr.h"


typedef struct _dataset {
    int    r;          // r instance number 
    int    c;          // c coefficient length
    int    *len;       // length of features of each instance
    int    *id;        // feature ids
    int    method;     // 1:L1, 2: L2
    double *val;       // feature values
    double *y;         // label of each instance
    double lambda;     // regularized paramenter
} Dataset;

/* ------------------------------------
 * brief  : free the dataset for LR
 * ds     : dataset pointer
 * ------------------------------------ */
void dataset_free(Dataset *ds) {
    if (ds != NULL) {
        if (ds->val != NULL) {
            free(ds->val);
            ds->val = NULL;
        }
        if (ds->id != NULL) {
            free(ds->id);
            ds->id = NULL;
        }
        if (ds->len != NULL) {
            free(ds->len);
            ds->len = NULL;
        }
        if (ds->y != NULL) {
            free(ds->y);
            ds->y = NULL;
        }
        free(ds);
    }
}

/* ----------------------------------------
 * brief : LR loss function value 
 * x     : current theta learned
 * _ds   : dataset for LR learn
 * return: current LR loss function value
 * ---------------------------------------- */
double lr_eval(double *x, void *_ds) {
    Dataset *ds = (Dataset *) _ds;
    double loss = 1.0, yest = 0.0, add = 0.0, regloss = 0.0;
    double *val = ds->val;
    int    *id  = ds->id;
    int    *len = ds->len;
    double *y   = ds->y;
    int offs =  0, i = 0, j = 0;

    for (offs = i = 0; i < ds->r; i++) {
        yest = 0.0;
        for (j = 0; j < len[i]; j++) {
            yest += val[offs + j] * x[id[offs + j]];
        }
        if (yest > 30.0){
            add = yest;
        }
        else if (yest > -30.0){
            add = log(1 + exp(yest));
        }
        else{
            add = 0.0;
        }
        if (y[i] > 0){
            add -= yest;
        }
      //if (ds->y[i] < 0.5) {
      //    yest = -yest;
      //}
      //if (yest < -30) {
      //    add = -yest;
      //} else if (yest > 30) {
      //    add = 0;
      //} else {
      //    add = log(1 + exp(-yest));
      //}
        loss += add;
        offs += len[i];
    }

    // add loss from regularization
    regloss = 0.0;
    if (ds->method == 2){       // for L2 Norm
        for (i = 0; i < ds->c; i++){
            regloss += x[i] * x[i];
        }
        loss += regloss * ds->lambda;
    }
    else if (ds->method == 1){  // for L1 Norm
        for (i = 0; i < ds->c; i++){
            if (x[i] > 0.0){
                regloss += x[i];
            }
            else if (x[i] < 0.0){
                regloss -= x[i];
            }
        }
        loss += regloss * ds->lambda;
    }
    return loss;
}
 
/* ----------------------------------------
 * brief : LR gradient function
 * x     : current theta learned
 * _ds   : dataset for LR learn
 * g     : gradient vector on current theta
 * return: current LR loss function value
 * ---------------------------------------- */
void lr_grad(double *x, void *_ds, double *g) {
    Dataset *ds = (Dataset *) _ds;
    double yest = 0.0, hx = 0.0;
    double *val = ds->val;
    double *y   = ds->y;
    int    *id  = ds->id;
    int    *len = ds->len;
    int i = 0, j = 0, offs = 0;
    memset(g, 0, sizeof(double) * ds->c);
    for (offs = i = 0; i < ds->r; i++) {
        yest = 0.0;
        for (j = 0; j < len[i]; j++) {
            yest += val[offs + j] * x[id[offs + j]];
        }
        if (yest < -30) {
            hx = 0.0;
        } else if (yest > 30) {
            hx = 1.0;
        } else {
            hx = 1.0 / (1.0 + exp(-yest));
        }
        for (j = 0; j < len[i]; j++) {
            g[id[offs + j]] += (hx - y[i]) * val[offs + j];
        }
        offs += len[i];
    }
    // Just for L2 Norm 
    if (ds->method == 2){
        for (i = 0; i < ds->c; i++){
            g[i] += ds->lambda * (x[i] + x[i]);
        }
    }
}

/* -----------------------------------------
 * brief  : LR function 
 * r      : r instance
 * c      : c coefficient
 * tlen   : number of data
 * len    : length of each instance
 * id     : features ids
 * val    : features values
 * y      : label of each instance
 * x      : return coefficient
 * ----------------------------------------- */
int lr(int r, int c, int tlen, int *len, int *id, double *val, double *y, double lambda, int method, double *x) {
    Dataset *ds = (Dataset *) malloc(sizeof(Dataset));
    ds->y       = (double  *) malloc(sizeof(double) * r);
    ds->val     = (double  *) malloc(sizeof(double) * tlen);
    ds->id      = (int *)     malloc(sizeof(int)    * tlen);
    ds->len     = (int *)     malloc(sizeof(int)    * r);
    ds->r       = r;
    ds->c       = c;
    ds->lambda  = lambda;
    ds->method  = method;
    memcpy(ds->y,   y,   sizeof(double) * r);
    memcpy(ds->val, val, sizeof(double) * tlen);
    memcpy(ds->id,  id,  sizeof(int)    * tlen);
    memcpy(ds->len, len, sizeof(int)    * r);
    if (method == 2){
        lbfgs(ds, lr_eval, lr_grad, 1e-9, 5, c, 1000, x);
    }
    else if (method == 1){
        owlqn(ds, lr_eval, lr_grad, lambda, 1e-9, 5, c, 1000, x);
    }
    dataset_free(ds);
    ds = NULL;
    return 0;
}

