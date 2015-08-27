/* ========================================================
 *   Copyright (C) 2015 All rights reserved.
 *   
 *   filename : er.c
 *   author   : liuzhiqiang01@baidu.com
 *   date     : 2015-08-27
 *   info     : exponential regression implementation
 *              using regression framework
 * ======================================================== */

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "newton_opt.h"
#include "regression.h"

// guassian regression implementation
typedef Regression ER;

/* ----------------------------------------
 * brief : ER gradient function
 * x     : current theta learned
 * _ds   : dataset for ER learn
 * g     : gradient vector on current theta
 * return: current ER loss function value
 * ---------------------------------------- */
void er_grad(double *x, void *_ds, double *g) {
    ER * er = (ER*) _ds;
    double yest = 0.0, hx = 0.0;
    double *val = er->train_ds->val;
    double *y   = er->train_ds->y;
    int    *id  = er->train_ds->ids;
    int    *len = er->train_ds->l;
    int     col = er->c;
    int     row = er->train_ds->r;
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
        if (val) {
            for (j = 0; j < len[i]; j++) {
                g[id[offs + j]] += (1.0 - y[i] / exp(yest)) * val[offs + j];
            }
        } else {
            for (j = 0; j < len[i]; j++) {
                g[id[offs + j]] += (1.0 - y[i] / exp(yest));
            }
        }
        offs += len[i];
    }
    // Just for L2 Norm 
    if (er->p.method == 2){
        for (i = 0; i < col; i++){
            g[i] += er->p.lambda * (x[i] + x[i]);
        }
    }
}

/* ----------------------------------------
 * brief : ER loss function value 
 * x     : current theta learned
 * _ds   : dataset for ER learn
 * return: current ER loss function value
 * ---------------------------------------- */
double er_eval(double *x, void *_ds) {
    ER * er = (ER *) _ds;
    double loss = 1.0, yest = 0.0, add = 0.0, regloss = 0.0;
    double *val = er->train_ds->val;
    int    *id  = er->train_ds->ids;
    int    *len = er->train_ds->l;
    double *y   = er->train_ds->y;
    int     row = er->train_ds->r;
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
        loss += y[i] / exp(yest) + yest;
        offs += len[i];
    }

    // add loss from regularization
    regloss = 0.0;
    if (er->p.method == 2){       // for L2 Norm
        for (i = 0; i < er->c; i++){
            regloss += x[i] * x[i];
        }
        loss += regloss * er->p.lambda;
    }
    else if (er->p.method == 1){  // for L1 Norm
        for (i = 0; i < er->c; i++){
            if (x[i] > 0.0){
                regloss += x[i];
            }
            else if (x[i] < 0.0){
                regloss -= x[i];
            }
        }
        loss += regloss * er->p.lambda;
    }
    return loss;
}

/* ----------------------------------------
 * brief : ER loss function value for test
 * x     : current theta result
 * _ds   : test data set
 * return: test loss value
 * ---------------------------------------- */
double er_eval_test(double *x, void *_ds) {
    ER * er = (ER *) _ds;
    double loss = 1.0, yest = 0.0, add = 0.0, regloss = 0.0;
    double *val = er->test_ds->val;
    int    *id  = er->test_ds->ids;
    int    *len = er->test_ds->l;
    double *y   = er->test_ds->y;
    int     row = er->test_ds->r;
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
        loss += y[i] / exp(yest) + yest;
        offs += len[i];
    }
    // no loss from regularization
    return loss;
}

/* ----------------------------------------
 * brief : report function for er
 * x0    : the last theta result
 * x1    : the current theta result
 * _ds   : the er model struct
 * ---------------------------------------- */
int er_repo(double *x0, double *x1, void *_ds) {
    ER * er = (ER *)_ds;
    double val1 = er_eval(x0, _ds);
    double val2 = er_eval(x1, _ds);
    if (fabs(val2 - val1) < er->p.ftoler){
        fprintf(stderr, "conv done exit\n");
        return 1;
    }
    int i = ++er->p.iterno;
    fprintf(stderr, "iter: %4d, train loss: %.10f", i, val2);
    if (er->test_ds){
        double test_loss = er_eval_test(x1, _ds);
        fprintf(stderr, ", test loss: %.10f", test_loss);
    }
    if (i % er->p.savestep == 0){
        memmove(er->x, x1, sizeof(double) * er->c);
        save_regression(er, i);
    }
    fprintf(stderr, "\n");
    return 0;
}
 
int  learn_regression(Regression * regression){
    ER * er = (ER*)regression;
    if (er->p.method == 2){
        lbfgs(er, er_eval, er_grad, er_repo, 5, er->c, er->p.niters, er->x);
    }
    else if (er->p.method == 1){
        owlqn(er, er_eval, er_grad, er_repo, 5, er->c, er->p.niters, er->p.lambda, er->x);
    }
    return 0;
}
