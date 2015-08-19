/* ========================================================
 *   Copyright (C) 2015 All rights reserved.
 *   
 *   filename : newton_opt.c
 *   author   : lizeming@baidu.com
 *              liuzhiqiang01@baidu.com
 *   date     : 2015-07-28
 *   info     : newton method for linear optimization
 * ======================================================== */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "newton_opt.h"

#define   EPS  1e-30

/* ---------------------------------
 * brief : sub gradient for L1 norm
 * n     : length of x
 * x     : current x
 * g     : ori grad
 * l1    : l1 norm para
 * sg    : sub grad
 * --------------------------------- */
static void sub_grad(int n, double *x, double *g, double l1, double *sg){
    memmove(sg, g, sizeof(double) * n);
    for (int i = 0; i < n; i++){
        if (x[i] > EPS) {
            sg[i] += l1;
        }
        else if (x[i] < -EPS){
            sg[i] -= l1;
        }
        else{
            if (sg[i] > l1){
                sg[i] -= l1;
            }
            else if (sg[i] < -l1){
                sg[i] += l1;
            }
            else{
                sg[i] = 0;
            }
        }
    }
}

/* -----------------------------------------
 * brief    : backtrack to find step for opt
 * data     : input data
 * eval_fn  : function for eval value
 * n        : length of x
 * x        : current x
 * g        : current grad if owlqn sub grad
 * dir      : current newton dir
 * newx     : the next x 
 * flag     : 0, lbfgs, 1 owlqn
 * ----------------------------------------- */
static double backtrack(void *data, EVAL_FN eval_fn, int n, double *x, double *g, double *dir, double *newx, int flag){
    memmove(newx, x, sizeof(double) * n);
    double f0 = 0.0, step = 1.0, gtd = 0.0, r = 0.0001;
    double sumD = 0.0;
    for (int i = 0; i < n; i++){
        gtd  += g[i]   * dir[i];
        sumD += dir[i] * dir[i];
    }
    sumD = sqrt(sumD);
    if (sumD < 0.125 && sumD > EPS) {
        step = 1.0 / sumD;
    }
    gtd *= r;
    f0 = eval_fn(x, data);
    for (int i = 0; i < 20; i++){
        for (int j = 0; j < n; j++){
            newx[j] = x[j] + step * dir[j];
            if (flag == 1){
                if (newx[j] * x[j] < 0) {newx[j] = 0;}
            }
        }
        if (eval_fn(newx, data) <= f0 + step * gtd){
            break;
        }
        step *= 0.5;
    }
    return step;
}

/* -------------------------------------------
 * brief : inv of H for bfgs
 * n     : length of x
 * s     : x1 - x0
 * y     : g1 - g0
 * b     : inv of H
 * ------------------------------------------- */
static void get_inv_h(int n, double *s, double *y, double *b) {
    double *ytb  = (double *) malloc(sizeof(double) * n);
    double *by   = (double *) malloc(sizeof(double) * n);
    double *sst  = (double *) malloc(sizeof(double) * n * n);
    double *byst = (double *) malloc(sizeof(double) * n * n);
    double *sytb = (double *) malloc(sizeof(double) * n * n);
    double sty = 0.0, ytby = 0.0, alpha = 0.0;
    int i = 0, j = 0, offs = 0;

    //STY = ST * Y
    for (i = 0; i < n; ++i) {
        sty += s[i] * y[i];
    }

    //BY = B * Y
    for (i = offs = 0; i < n; ++i, offs += n) {
        by[i] = 0;
        for (j = 0; j < n; ++j) {
            by[i] += b[offs + j] * y[j];
        }
    }

    //BYST = BY * ST
    for (i = offs = 0; i < n; ++i, offs += n) {
        for (j = 0; j < n; ++j) {
            byst[offs + j] = by[i] * s[j];
        }
    }

    //YTB = YT * B
    for (i = offs = 0; i < n; i++, offs += n){
        ytb[i] = 0;
        for (j = 0; j < n; j++){
            ytb[j] += y[i] * b[offs + j];
        }
    }

    //YTBY = YTB * Y
    for (i = 0; i < n; ++i) {
        ytby += ytb[i] * y[i];
    }

    //SYTB = S * YTB
    for (i = offs = 0; i < n; ++i, offs += n) {
        for (j = 0; j < n; ++j) {
            sytb[offs + j] = s[i] * ytb[j];
        }
    }
  
    //SST = S * ST
    for (i = offs = 0; i < n; ++i, offs += n) {
        for (j = 0; j < n; ++j) {
            sst[offs + j] = s[i] * s[j];
        }
    }

    alpha = 1. / sty + ytby / sty / sty;
    for (i = 0; i < n * n; ++i) {
        b[i] += alpha * sst[i] - (byst[i] + sytb[i]) / sty;
    }

    free(ytb);
    free(by);
    free(sst);
    free(byst);
    free(sytb);
}

/* -----------------------------------------------
 * brief   : get Hg for lbfgs or owlqn
 * n       : length of x
 * g       : current grad if owlqn sub grad
 * s       : last x1 - x0 sequences
 * y       : last g1 - g0 sequences
 * m       : most length of s or y
 * pos     : start index of s or y
 * k       : use lastl k of s or y
 * hgini   : hg init value
 * hg      : Hg result
 * ----------------------------------------------- */
static void get_hg(int n, double *g,  double *s, double *y, int m, int pos, int k, double hgini, double *hg){
    double *alpha = (double*)malloc(sizeof(double) * k);
    double *r     = (double*)malloc(sizeof(double) * k);
    double *q     = (double*)malloc(sizeof(double) * n);
    double yts = 0.0, stq = 0.0, yhg = 0.0, beta = 0.0;
    int *id = (int*)malloc(sizeof(int) * k);

    // last k s, y sequences' index
    for (int i = 0; i < k; i++, pos++){
        id[i] = pos < m ? pos : pos - m;
    }
    for (int i = 0; i < k; i++){
        int offs = id[i] * n;
        for (int j = 0; j < n; j++){
            yts += s[offs + j] * y[offs + j];
        }
        r[i] = 1.0 / yts;
    }
    memmove(q, g, sizeof(double) * n);
    // step 1 for two loops
    for (int i = k - 1; i >= 0; i--){
        stq = 0.0;
        int offs = id[i] * n;
        for (int j = 0; j < n; j++){
            stq += s[offs + j] * q[j];
        }
        alpha[i] = r[i] * stq;
        for (int j = 0; j < n; j++){
            q[j] -= alpha[i] * y[offs + j];
        }
    }
    
    //HgInit
    for (int i = 0; i < n; i++){
        hg[i] = hgini * q[i];
    }

    //step 2 for two loops
    for (int i = 0; i < k; i++){
        int offs = id[i] * n;
        yhg = 0.0;
        for (int j = 0; j < n; j++){
            yhg += y[offs + j] * hg[j];
        }
        beta = r[i] * yhg;
        for (int j = 0; j < n; j++){
            hg[j] += s[offs + j] * (alpha[i] - beta);
        }
    }

    free(id);
    free(alpha);
    free(r);
    free(q);
}

int bfgs(void *data, EVAL_FN eval_fn, GRAD_FN grad_fn, double ftol, int n, int it, double *retx) {
    double *x0  = (double *) malloc(sizeof(double) * n);
    double *x1  = (double *) malloc(sizeof(double) * n);
    double *g0  = (double *) malloc(sizeof(double) * n);
    double *g1  = (double *) malloc(sizeof(double) * n);
    double *dir = (double *) malloc(sizeof(double) * n);
    double *b   = (double *) malloc(sizeof(double) * n * n);
    double *s   = (double *) malloc(sizeof(double) * n);
    double *y   = (double *) malloc(sizeof(double) * n);
    double *t   = NULL;
    double fval = 0.0, prefval = 0.0, lambda = 0.0;
    int  i = 0, j = 0, k = 0, offs = 0;

    memset(x0, 0, sizeof(double) * n);
    fval = eval_fn(x0, data);
    grad_fn(x0, data, g0);

    for (i = offs = 0; i < n; i++, offs += n){
        b[offs + i] = 1.0;
    }

    for (i = 0; i < it; i++){
        // newton dir : -H*g
        memset(dir, 0, sizeof(double) * n);
        for (j = offs = 0; j < n; j++, offs += n){
            for (k = 0; k < n; k++){
                dir[j] -= b[offs + k] * g0[k];
            }
        }
        lambda = backtrack(data, eval_fn, n, x0, g0, dir, x1, 0);
        prefval = fval;
        fval = eval_fn(x1, data);
        if (fabs(fval - prefval) < ftol || i == (it - 1)){
            break;
        }
        grad_fn(x1, data, g1);
        for(j = 0; j < n; j++){
            s[j] = x1[j] - x0[j];
            y[j] = g1[j] - g0[j];
        }
        t = g0; g0 = g1; g1 = t;
        t = x0; x0 = x1; x1 = t;
        get_inv_h(n, s, y, b);
    }
    memmove(retx, x1, sizeof(double) * n);

    free(x0);
    free(x1);
    free(g0);
    free(g1);
    free(dir);
    free(b);
    free(s);
    free(y);
    return 0;
}

int lbfgs(void *data, EVAL_FN eval_fn, GRAD_FN grad_fn, REPO_FN repo_fn, int m, int n, int it, double *retx){
    double *x0  = (double *) malloc(sizeof(double) * n);
    double *x1  = (double *) malloc(sizeof(double) * n);
    double *g0  = (double *) malloc(sizeof(double) * n);
    double *g1  = (double *) malloc(sizeof(double) * n);
    double *s   = (double *) malloc(sizeof(double) * n);
    double *y   = (double *) malloc(sizeof(double) * n);
    double *sm  = (double *) malloc(sizeof(double) * m * n);
    double *ym  = (double *) malloc(sizeof(double) * m * n);
    double *hg  = (double *) malloc(sizeof(double) * n);
    double *t   = NULL;
    double fval = 0.0, prefval = 0.0, hgini = 0.0, yty = 0.0, lambda = 0.0;
    int i = 0, j = 0, offs = 0, pos = 0, block = m * n;

    // First iteration
    memset(x0, 0, sizeof(double) * n);
    grad_fn(x0, data, g0);
    for (i = 0; i < n; i++){
        hg[i] = -g0[i];
    }
    lambda = backtrack(data, eval_fn, n, x0, g0, hg, x1, 0);
    grad_fn(x1, data, g1);

    // Iteration
    for (i = 1, offs = 0; i < it; i++, offs += n){
        if (repo_fn && repo_fn(x0, x1, data)){
            break;
        }
        hgini = yty = 0;
        offs = offs < block ? offs : (offs - block);
        for (j = 0; j < n; j++){
            sm[offs + j] = s[j] = x1[j] - x0[j];
            ym[offs + j] = y[j] = g1[j] - g0[j];
            hgini += s[j] * y[j];
            yty   += y[j] * y[j];
        }
        if (yty < EPS && yty > -EPS) {
            break;
        }
        hgini /= yty;
        t = x0, x0 = x1, x1 = t;
        t = g0, g0 = g1, g1 = t;
        pos = i > m ? (i % m) : 0;
        get_hg(n, g0, sm, ym, m, pos, (i < m ? i : m), hgini, hg);

        // newton dir = -Hg
        for (j = 0; j < n; j++){
            hg[j] = -hg[j];
        }

        lambda = backtrack(data, eval_fn, n, x0, g0, hg, x1, 0);
        grad_fn(x1, data, g1);
    }
    memmove(retx, x1, sizeof(double) * n);

    free(x0);
    free(x1);
    free(g0);
    free(g1);
    free(s);
    free(y);
    free(sm);
    free(ym);
    free(hg);
    return 0;
}

int owlqn(void *data, EVAL_FN eval_fn, GRAD_FN grad_fn, REPO_FN repo_fn, int m, int n, int it, double l1, double *retx){
    double *x0  = (double *) malloc(sizeof(double) * n);
    double *x1  = (double *) malloc(sizeof(double) * n);
    double *g0  = (double *) malloc(sizeof(double) * n);
    double *g1  = (double *) malloc(sizeof(double) * n);
    double *s   = (double *) malloc(sizeof(double) * n);
    double *y   = (double *) malloc(sizeof(double) * n);
    double *sg  = (double *) malloc(sizeof(double) * n);
    double *sm  = (double *) malloc(sizeof(double) * m * n);
    double *ym  = (double *) malloc(sizeof(double) * m * n);
    double *hg  = (double *) malloc(sizeof(double) * n);
    double *t   = NULL;
    double fval = 0.0, prefval = 0.0, hgini = 0.0, yty = 0.0, lambda = 0.0;
    int i = 0, j = 0, offs = 0, pos = 0, block = m * n;

    // first iteration
    memset(x0, 0, sizeof(double) * n);
    grad_fn(x0, data, g0);
    sub_grad(n, x0, g0, l1, sg);
    for (i = 0; i < n; i++){
        hg[i] = -sg[i];
    }
    lambda = backtrack(data, eval_fn, n, x0, sg, hg, x1, 1);
    grad_fn(x1, data, g1);
    sub_grad(n, x1, g1, l1, sg);

    // Iteration
    for (i = 1, offs = 0; i < it; i++, offs += n){
        if (repo_fn && repo_fn(x0, x1, data)){
            break;
        }
        hgini = yty = 0;
        offs = offs < block ? offs : (offs - block);
        for (j = 0; j < n; j++){
            sm[offs + j] = s[j] = x1[j] - x0[j];
            ym[offs + j] = y[j] = g1[j] - g0[j];
            hgini += s[j] * y[j];
            yty   += y[j] * y[j];
        }
        if (yty < EPS && yty > -EPS) {
            break;
        }
        hgini /= yty;
        t = x0, x0 = x1, x1 = t;
        t = g0, g0 = g1, g1 = t;
        pos = i > m ? (i % m) : 0;
        get_hg(n, sg, sm, ym, m, pos, (i < m ? i : m), hgini, hg);

        // newton dir = -hg
        for (j = 0; j < n; j++){
            if (sg[j] * hg[j] < 0){
                hg[j] = 0;
            }
            else{
                hg[j] = -hg[j];
            }
        }

        lambda = backtrack(data, eval_fn, n, x0, sg, hg, x1, 1);
        for (j = 0; j < n; j++){
            if (x0[j] * x1[j] < 0){
                x1[j] = 0;
            }
        }
        grad_fn(x1, data, g1);
        sub_grad(n, x1, g1, l1, sg);
    }
    memmove(retx, x1, sizeof(double) * n);

    free(x0);
    free(x1);
    free(g0);
    free(g1);
    free(sg);
    free(s);
    free(y);
    free(sm);
    free(ym);
    free(hg);
    return 0;
}
