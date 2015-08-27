/* ========================================================
 *   Copyright (C) 2015 All rights reserved.
 *   
 *   filename : lr.c
 *   author   : liuzhiqiang01@baidu.com
 *   date     : 2015-08-27
 *   info     : LR implementation
 *              Using regression framework
 * ======================================================== */

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "auc.h"
#include "newton_opt.h"
#include "regression.h"

// LR implementation
typedef Regression LR;
typedef RDS        LRDS;

/* ----------------------------------------
 * brief : LR gradient function
 * x     : current theta learned
 * _ds   : dataset for LR learn
 * g     : gradient vector on current theta
 * return: current LR loss function value
 * ---------------------------------------- */
void lr_grad(double *x, void *_ds, double *g) {
    LR * lr = (LR*) _ds;
    double yest = 0.0, hx = 0.0;
    double *val = lr->train_ds->val;
    double *y   = lr->train_ds->y;
    int    *id  = lr->train_ds->ids;
    int    *len = lr->train_ds->l;
    int     col = lr->c;
    int     row = lr->train_ds->r;
    int i = 0, j = 0, offs = 0;
    memset(g, 0, sizeof(double) * col);
    for (offs = i = 0; i < row; i++) {
        yest = 0.0;
        if (val) {
            for (j = 0; j < len[i]; j++) {
                yest += val[offs + j] * x[id[offs + j]];
            }
        } else {
            for (j = 0; j < len[i]; j++) {
                yest += x[id[offs + j]];
            }
        }
        if (yest < -30) {
            hx = 0.0;
        } else if (yest > 30) {
            hx = 1.0;
        } else {
            hx = 1.0 / (1.0 + exp(-yest));
        }
        if (val) {
            for (j = 0; j < len[i]; j++) {
                g[id[offs + j]] += (hx - y[i]) * val[offs + j];
            }
        } else {
            for (j = 0; j < len[i]; j++) {
                g[id[offs + j]] += (hx - y[i]);
            }
        }
        offs += len[i];
    }
    // Just for L2 Norm 
    if (lr->p.method == 2){
        for (i = 0; i < col; i++){
            g[i] += lr->p.lambda * (x[i] + x[i]);
        }
    }
}

/* ----------------------------------------
 * brief : LR loss function value 
 * x     : current theta learned
 * _ds   : dataset for LR learn
 * return: current LR loss function value
 * ---------------------------------------- */
double lr_eval(double *x, void *_ds) {
    LR * lr = (LR *) _ds;
    double loss = 1.0, yest = 0.0, add = 0.0, regloss = 0.0;
    double *val = lr->train_ds->val;
    int    *id  = lr->train_ds->ids;
    int    *len = lr->train_ds->l;
    double *y   = lr->train_ds->y;
    int     row = lr->train_ds->r;
    int offs =  0, i = 0, j = 0;

    for (offs = i = 0; i < row; i++) {
        yest = 0.0;
        if (val) {
            for (j = 0; j < len[i]; ++j) {
                yest += val[offs + j] * x[id[offs + j]];
            }
        } else {
            for (j = 0; j < len[i]; ++j) {
                yest += x[id[offs + j]];
            }
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
        loss += add;
        offs += len[i];
    }

    // add loss from regularization
    regloss = 0.0;
    if (lr->p.method == 2){       // for L2 Norm
        for (i = 0; i < lr->c; i++){
            regloss += x[i] * x[i];
        }
        loss += regloss * lr->p.lambda;
    }
    else if (lr->p.method == 1){  // for L1 Norm
        for (i = 0; i < lr->c; i++){
            if (x[i] > 0.0){
                regloss += x[i];
            }
            else if (x[i] < 0.0){
                regloss -= x[i];
            }
        }
        loss += regloss * lr->p.lambda;
    }
    return loss;
}

/* --------------------------------------
 * brief : calculating auc value for LR
 * x     : current theta result
 * _ds   : dataset used for scores
 * -------------------------------------- */
double lr_auc(double *x, void *_ds){
    LRDS * ds = (LRDS*)_ds;
    double  *y  = ds->y;
    int     row = ds->r;
    double *val = ds->val;
    int    *id  = ds->ids;
    int    *len = ds->l;
    double * s = (double*)malloc(sizeof(double) * row);
    memset(s, 0, sizeof(double) * row);
    int offs =  0, i = 0, j = 0;
    for (i = offs = 0; i < row; i++){
        if (val){
            for (j = 0; j < len[i]; j++){
                s[i] += val[offs + j] * x[id[offs + j]]; 
            }
        }
        else{
            for (j = 0; j < len[i]; j++){
                s[i] += x[id[offs + j]]; 
            }
        }
        offs += len[i];
    }
    double auc_v = auc(ds->r, s, y);
    free(s); s = NULL;
    return auc_v;
}

/* ----------------------------------------
 * brief : report function for lr
 * x0    : the last theta result
 * x1    : the current theta result
 * _ds   : the lr model struct
 * ---------------------------------------- */
int lr_repo(double *x0, double *x1, void *_ds) {
    LR * lr = (LR *)_ds;
    double val1 = lr_eval(x0, _ds);
    double val2 = lr_eval(x1, _ds);
    if (fabs(val2 - val1) < lr->p.ftoler){
        fprintf(stderr, "conv done exit\n");
        return 1;
    }
    int i = ++lr->p.iterno;
    fprintf(stderr, "iter: %4d, loss: %.10f", i, val2);
    if (i % lr->p.savestep == 0){
        double auc = lr_auc(x1, lr->train_ds);
        fprintf(stderr, ",train_auc: %.10f", auc);
        if (lr->test_ds){
            auc = lr_auc(x1, lr->test_ds);
            fprintf(stderr, ",test_auc: %.10f", auc);
        }
        memmove(lr->x, x1, sizeof(double) * lr->c);
        save_regression(lr, i);
    }
    fprintf(stderr, "\n");
    return 0;
}
 
int  learn_regression(Regression * regression){
    LR * lr = (LR*)regression;
    if (lr->p.method == 2){
        lbfgs(lr, lr_eval, lr_grad, lr_repo, 5, lr->c, lr->p.niters, lr->x);
    }
    else if (lr->p.method == 1){
        owlqn(lr, lr_eval, lr_grad, lr_repo, 5, lr->c, lr->p.niters, lr->p.lambda, lr->x);
    }
    return 0;

}
