/* ========================================================
 *   Copyright (C) 2015 All rights reserved.
 *   
 *   filename : pr.c
 *   author   : liuzhiqiang01@baidu.com
 *   date     : 2015-08-27
 *   info     : possion regression implementation
 * ======================================================== */

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "newton_opt.h"
#include "regression.h"


// PR implementation
typedef Regression PR;
typedef RDS        PRDS;

/* ----------------------------------------
 * brief : PR gradient function
 * x     : current theta learned
 * _ds   : dataset for PR learn
 * g     : gradient vector on current theta
 * return: current PR loss function value
 * ---------------------------------------- */
void pr_grad(double *x, void *_ds, double *g) {
    PR * pr = (PR*) _ds;
    double yest = 0.0, hx = 0.0;
    double *val = pr->train_ds->val;
    double *y   = pr->train_ds->y;
    int    *id  = pr->train_ds->ids;
    int    *len = pr->train_ds->l;
    int     col = pr->c;
    int     row = pr->train_ds->r;
    int i = 0, j = 0, offs = 0;
    memset(g, 0, sizeof(double) * col);
    for (offs = i = 0; i < row; i++) {
        yest = 0.0;
        if (val) {
            for (j = 0; j < len[i]; j++) {
                yest += val[offs + j] * x[id[offs + j]];
            }
            for (j = 0; j < len[i]; j++){
                g[id[offs + j]] += (exp(yest) - y[i]) * val[offs + j];
            }
        } else {
            for (j = 0; j < len[i]; j++) {
                yest += x[id[offs + j]];
            }
            for (j = 0; j < len[i]; j++) {
                g[id[offs + j]] += (exp(yest) - y[i]);
            }
        }
        offs += len[i];
    }
    // Just for L2 Norm 
    if (pr->p.method == 2){
        for (i = 0; i < col; i++){
            g[i] += pr->p.lambda * (x[i] + x[i]);
        }
    }
}

/* ----------------------------------------
 * brief : PR loss function value 
 * x     : current theta learned
 * _ds   : dataset for PR learn
 * return: current PR loss function value
 * ---------------------------------------- */
double pr_eval(double *x, void *_ds) {
    PR * pr = (PR *) _ds;
    double loss = 1.0, yest = 0.0, add = 0.0, regloss = 0.0;
    double *val = pr->train_ds->val;
    int    *id  = pr->train_ds->ids;
    int    *len = pr->train_ds->l;
    double *y   = pr->train_ds->y;
    int     row = pr->train_ds->r;
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
        loss += exp(yest) - y[i] * yest;
        offs += len[i];
    }

    // add loss from regularization
    regloss = 0.0;
    if (pr->p.method == 2){       // for L2 Norm
        for (i = 0; i < pr->c; i++){
            regloss += x[i] * x[i];
        }
        loss += regloss * pr->p.lambda;
    }
    else if (pr->p.method == 1){  // for L1 Norm
        for (i = 0; i < pr->c; i++){
            if (x[i] > 0.0){
                regloss += x[i];
            }
            else if (x[i] < 0.0){
                regloss -= x[i];
            }
        }
        loss += regloss * pr->p.lambda;
    }
    return loss;
}

/* ----------------------------------------
 * brief : PR loss function value for test
 * x     : current theta learned
 * _ds   : test data set
 * return: test loss value
 * ---------------------------------------- */
double pr_eval_test(double *x, void *_ds) {
    PR * pr = (PR *) _ds;
    double loss = 1.0, yest = 0.0, add = 0.0, regloss = 0.0;
    double *val = pr->test_ds->val;
    int    *id  = pr->test_ds->ids;
    int    *len = pr->test_ds->l;
    double *y   = pr->test_ds->y;
    int     row = pr->test_ds->r;
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
        loss += exp(yest) - y[i] * yest;
        offs += len[i];
    }

    // no loss from regularization
    return loss;
}

int pr_repo(double *x0, double *x1, void *_ds) {
    PR * pr = (PR *)_ds;
    double val1 = pr_eval(x0, _ds);
    double val2 = pr_eval(x1, _ds);
    if (fabs(val2 - val1) < pr->p.ftoler){
        fprintf(stderr, "conv done exit\n");
        return 1;
    }
    int i = ++pr->p.iterno;
    fprintf(stderr, "iter: %4d, train loss: %.10f", i, val2);
    if (pr->test_ds){
        double test_loss = pr_eval_test(x1, _ds);
        fprintf(stderr, ", test loss: %.10f", test_loss);
    }
    if (i % pr->p.savestep == 0){
        memmove(pr->x, x1, sizeof(double) * pr->c);
        save_regression(pr, i);
    }
    fprintf(stderr, "\n");
    return 0;
}
 

int  learn_regression(Regression * regression){
    PR * pr = (PR*)regression;
    if (pr->p.method == 2){
        lbfgs(pr, pr_eval, pr_grad, pr_repo, 5, pr->c, pr->p.niters, pr->x);
    }
    else if (pr->p.method == 1){
        owlqn(pr, pr_eval, pr_grad, pr_repo, 5, pr->c, pr->p.niters, pr->p.lambda, pr->x);
    }
    return 0;

}
