/* ========================================================
 *   Copyright (C) 2015 All rights reserved.
 *   
 *   filename : gr.c
 *   author   : liuzhiqiang01@baidu.com
 *   date     : 2015-08-27
 *   info     : guassian regression implementation
 *              using regression framework
 * ======================================================== */

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "newton_opt.h"
#include "regression.h"

// guassian regression implementation
typedef Regression GR;

/* ----------------------------------------
 * brief : GR gradient function
 * x     : current theta learned
 * _ds   : dataset for GR learn
 * g     : gradient vector on current theta
 * return: current GR loss function value
 * ---------------------------------------- */
void gr_grad(double *x, void *_ds, double *g) {
    GR * gr = (GR*) _ds;
    double yest = 0.0, hx = 0.0;
    double *val = gr->train_ds->val;
    double *y   = gr->train_ds->y;
    int    *id  = gr->train_ds->ids;
    int    *len = gr->train_ds->l;
    int     col = gr->c;
    int     row = gr->train_ds->r;
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
                g[id[offs + j]] += (y[i] - yest) * val[offs + j];
            }
        } else {
            for (j = 0; j < len[i]; j++) {
                g[id[offs + j]] += (y[i] - yest);
            }
        }
        offs += len[i];
    }
    // Just for L2 Norm 
    if (gr->p.method == 2){
        for (i = 0; i < col; i++){
            g[i] += gr->p.lambda * (x[i] + x[i]);
        }
    }
}

/* ----------------------------------------
 * brief : GR loss function value 
 * x     : current theta learned
 * _ds   : dataset for GR learn
 * return: current GR loss function value
 * ---------------------------------------- */
double gr_eval(double *x, void *_ds) {
    GR * gr = (GR *) _ds;
    double loss = 1.0, yest = 0.0, add = 0.0, regloss = 0.0;
    double *val = gr->train_ds->val;
    int    *id  = gr->train_ds->ids;
    int    *len = gr->train_ds->l;
    double *y   = gr->train_ds->y;
    int     row = gr->train_ds->r;
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
        loss += (y[i] - yest) * (y[i] - yest);
        offs += len[i];
    }

    // add loss from regularization
    regloss = 0.0;
    if (gr->p.method == 2){       // for L2 Norm
        for (i = 0; i < gr->c; i++){
            regloss += x[i] * x[i];
        }
        loss += regloss * gr->p.lambda;
    }
    else if (gr->p.method == 1){  // for L1 Norm
        for (i = 0; i < gr->c; i++){
            if (x[i] > 0.0){
                regloss += x[i];
            }
            else if (x[i] < 0.0){
                regloss -= x[i];
            }
        }
        loss += regloss * gr->p.lambda;
    }
    return loss;
}

/* ----------------------------------------
 * brief : GR loss function value for test
 * x     : current theta result
 * _ds   : test data set
 * return: test loss value
 * ---------------------------------------- */
double gr_eval_test(double *x, void *_ds) {
    GR * gr = (GR *) _ds;
    double loss = 1.0, yest = 0.0, add = 0.0, regloss = 0.0;
    double *val = gr->test_ds->val;
    int    *id  = gr->test_ds->ids;
    int    *len = gr->test_ds->l;
    double *y   = gr->test_ds->y;
    int     row = gr->test_ds->r;
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
        loss += (y[i] - yest) * (y[i] - yest);
        offs += len[i];
    }
    // no loss from regularization
    return loss;
}

/* ----------------------------------------
 * brief : report function for gr
 * x0    : the last theta result
 * x1    : the current theta result
 * _ds   : the gr model struct
 * ---------------------------------------- */
int gr_repo(double *x0, double *x1, void *_ds) {
    GR * gr = (GR *)_ds;
    double val1 = gr_eval(x0, _ds);
    double val2 = gr_eval(x1, _ds);
    if (fabs(val2 - val1) < gr->p.ftoler){
        fprintf(stderr, "conv done exit\n");
        return 1;
    }
    int i = ++gr->p.iterno;
    fprintf(stderr, "iter: %4d, train loss: %.10f", i, val2);
    if (gr->test_ds){
        double test_loss = gr_eval_test(x1, _ds);
        fprintf(stderr, ", test loss: %.10f", test_loss);
    }
    if (i % gr->p.savestep == 0){
        memmove(gr->x, x1, sizeof(double) * gr->c);
        save_regression(gr, i);
    }
    fprintf(stderr, "\n");
    return 0;
}
 
int  learn_regression(Regression * regression){
    GR * gr = (GR*)regression;
    if (gr->p.method == 2){
        lbfgs(gr, gr_eval, gr_grad, gr_repo, 5, gr->c, gr->p.niters, gr->x);
    }
    else if (gr->p.method == 1){
        owlqn(gr, gr_eval, gr_grad, gr_repo, 5, gr->c, gr->p.niters, gr->p.lambda, gr->x);
    }
    return 0;
}
