/* ========================================================
 *   Copyright (C) 2016 All rights reserved.
 *   
 *   filename : tm.c
 *   author   : liuzhiqiangruc@126.com
 *   date     : 2016-07-15
 *   info     : 
 * ======================================================== */

#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "hash.h"
#include "str.h"
#include "tm.h"

typedef struct {
    unsigned short prev;
    unsigned short next;
    unsigned int   count;
}ModelEle;

struct _tm{
    unsigned int d;
    unsigned int v;
    unsigned int t;
    char (*id_d_map)[KEY_SIZE];
    char (*id_v_map)[KEY_SIZE];
    int  (*tokens)[4];
    int   * doc_entry;
    ModelEle * nd;
    ModelEle * nw;
    int   * nkw;
    TMConfig * tmc;
};

static void malloc_space(TM * tm){
    int k = tm->tmc->get_k(tm->tmc);
    tm->id_d_map  = (char(*)[KEY_SIZE])calloc(tm->d,  sizeof(char[KEY_SIZE]));
    tm->id_v_map  = (char(*)[KEY_SIZE])calloc(tm->v,  sizeof(char[KEY_SIZE]));
    tm->tokens    = (int(*)[4])        calloc(tm->t,  sizeof(int[4]));
    tm->nkw       = (int *)            calloc(k + 1,  sizeof(int));
    tm->doc_entry = (int *)            malloc(tm->d * sizeof(int));
    tm->nd        = (ModelEle*)        calloc(tm->d * (k + 1), sizeof(ModelEle));
    tm->nw        = (ModelEle*)        calloc(tm->v * (k + 1), sizeof(ModelEle));
    memset(tm->doc_entry, -1, tm->d * sizeof(int));
}

static void fullfill_param(TM * tm){
    int i, d, v, t, k, o, p;
    k = tm->tmc->get_k(tm->tmc);
    for (i = 0; i < tm->t; i++){
        d = tm->tokens[i][0];
        v = tm->tokens[i][1];
        t = tm->tokens[i][2];
        tm->nd[d * (k + 1) + t].count += 1;
        tm->nw[v * (k + 1) + t].count += 1;
        tm->nkw[t] += 1;
    }
    for (d = 0; d < tm->d; d++){
        o = d * (k + 1);
        p = 0;
        for (t = 1; t <= k; t++){
            if (tm->nd[o + t].count > 0){
                tm->nd[o + p].next = t;
                tm->nd[o + t].prev = p;
                p = t;
            }
        }
        tm->nd[o + p].next = 0;
        tm->nd[o].prev = p;
    }
    for (v = 0; v < tm->v; v++){
        o = v * (k + 1);
        p = 0;
        for (t = 1; t <= k; t++){
            if (tm->nw[o + t].count > 0){
                tm->nw[o + p].next = t;
                tm->nw[o + t].prev = p;
                p = t;
            }
        }
        tm->nw[o + p].next = 0;
        tm->nw[o].prev = p;
    }
    tm_save(tm, 0);
}

static double init_e(TM * tm, double * pse){
    int t, k;
    double e , alpha, beta, ab, vb;
    k = tm->tmc->get_k(tm->tmc);
    alpha = tm->tmc->get_alpha(tm->tmc);
    beta  = tm->tmc->get_beta(tm->tmc);
    ab    = alpha * beta;
    vb    = beta  * tm->v;
    e     = 0.0;
    for (t = 1; t < k + 1; t++){
        pse[t] = ab / (vb + tm->nkw[t]);
        e += pse[t];
    }
    return e;
}

static double update_e(TM * tm, double * pse, int t, int delta){
    double tmp, beta, alpha, ab, vb;
    alpha = tm->tmc->get_alpha(tm->tmc);
    beta  = tm->tmc->get_beta(tm->tmc);
    ab    = alpha * beta;
    vb    = beta  * tm->v;
    tmp   = ab / (vb + tm->nkw[t]);
    tmp  -= ab / (vb + tm->nkw[t] + delta);
    pse[t] += tmp;
    return tmp;
}

static double init_f(TM * tm, double * psd, int d){
    int t, k, o;
    double f, beta, vb;
    k    = tm->tmc->get_k(tm->tmc);
    beta = tm->tmc->get_beta(tm->tmc);
    vb   = beta * tm->v;
    o    = d * (k + 1);
    f    = 0.0;
    t    = tm->nd[o].next;
    while (t){
        psd[t] = beta * tm->nd[o + t].count / (vb + tm->nkw[t]);
        f += psd[t];
        t = tm->nd[o + t].next;
    }
    return f;
}

static double update_f(TM * tm, double * psd, int d, int t, int delta){
    int k, o;
    double beta, vb, tmp;
    k    = tm->tmc->get_k(tm->tmc);
    o    = d * (k + 1);
    beta = tm->tmc->get_beta(tm->tmc);
    vb   = beta * tm->v;
    tmp  = beta * tm->nd[o + t].count / (vb + tm->nkw[t]);
    tmp -= beta * (tm->nd[o + t].count + delta) / (vb + tm->nkw[t] + delta);
    psd[t] += tmp;
    return tmp;
}

static double init_g(TM * tm, double * psv, int d, int v){
    int t, k, doffs, voffs;
    double g, beta, alpha, vb;
    k     = tm->tmc->get_k(tm->tmc);
    beta  = tm->tmc->get_beta(tm->tmc);
    alpha = tm->tmc->get_alpha(tm->tmc);
    vb    = beta * tm->v;
    doffs = d * (k + 1);
    voffs = v * (k + 1);
    g     = 0.0;
    t     = tm->nw[voffs].next;
    while (t){
        psv[t] = (alpha + tm->nd[doffs + t].count) * tm->nw[voffs + t].count / (vb + tm->nkw[t]);
        g += psv[t];
        t = tm->nw[voffs + t].next;
    }
    return g;
}

static void del_off_tp(TM * tm, int d, int v, int t){
    int doffs, voffs, k, p, n;
    k = tm->tmc->get_k(tm->tmc);
    doffs = d * (k + 1);
    voffs = v * (k + 1);
    tm->nd[doffs + t].count -= 1;
    tm->nw[voffs + t].count -= 1;
    tm->nkw[t] -= 1;
    if (tm->nd[doffs + t].count == 0){
        p = tm->nd[doffs + t].prev;
        n = tm->nd[doffs + t].next;
        tm->nd[doffs + p].next = n;
        tm->nd[doffs + n].prev = p;
    }
    if (tm->nw[voffs + t].count == 0){
        p = tm->nw[voffs + t].prev;
        n = tm->nw[voffs + t].next;
        tm->nw[voffs + p].next = n;
        tm->nw[voffs + n].prev = p;
    }
}

static void add_on_tp(TM * tm, int d, int v, int t) {
    int doffs, voffs, k, p, n;
    k = tm->tmc->get_k(tm->tmc);
    doffs = d * (k + 1);
    voffs = v * (k + 1);
    if (tm->nd[doffs + t].count == 0){
        tm->nd[doffs + tm->nd[doffs].prev].next = t;
        tm->nd[doffs + t].prev = tm->nd[doffs].prev;
        tm->nd[doffs + t].next = 0;
        tm->nd[doffs].prev = t;
    }
    if (tm->nw[voffs + t].count == 0){
        tm->nw[voffs + tm->nw[voffs].prev].next = t;
        tm->nw[voffs + t].prev = tm->nw[voffs].prev;
        tm->nw[voffs + t].next = 0;
        tm->nw[voffs].prev = t;
    }
    tm->nd[doffs + t].count += 1;
    tm->nw[voffs + t].count += 1;
    tm->nkw[t] += 1;
}

static int sample_k(TM * tm, double * pse
                           , double * psd
                           , double * psv
                           , double e
                           , double f
                           , double g
                           , double r
                           , int d
                           , int v){
    int t, k, doffs, voffs;
    double tmp;
    k = tm->tmc->get_k(tm->tmc);
    doffs = d * (k + 1);
    voffs = v * (k + 1);
    tmp = 0.0;
    if (r < e){
        for (t = 1; t < k + 1; t++){
            tmp += pse[t];
            if (tmp > r){
                break;
            }
        }
    }
    else if (r < e + f){
        r -= e;
        t = tm->nd[doffs].next;
        while (t){
            tmp += psd[t];
            if (tmp > r){
                break;
            }
            t = tm->nd[doffs + t].next;
        }
    }
    else{
        r -= (e + f);
        t = tm->nw[voffs].next;
        while (t){
            tmp += psv[t];
            if (tmp > r){
                break;
            }
            t = tm->nw[voffs + t].next;
        }
    }
    return t;
}

static int gibbs_sample(TM * tm){
    int i, d, v, t, k;
    k = tm->tmc->get_k(tm->tmc);
    double *pse  = (double*)calloc(k + 1, sizeof(double));
    double *psd  = (double*)calloc(k + 1, sizeof(double));
    double *psv  = (double*)calloc(k + 1, sizeof(double));
    double e, f, g, r;
    e = init_e(tm, pse);
    for (d = 0; d < tm->d; d++){
        f = init_f(tm, psd, d);
        i = tm->doc_entry[d];
        while (i != -1){
            v = tm->tokens[i][1];
            t = tm->tokens[i][2];
            del_off_tp(tm, d, v, t);
            e += update_e(tm, pse, t, 1);
            f += update_f(tm, psd, d, t, 1);
            g  = init_g(tm, psv, d, v);
            r = (e + f + g) * (0.1 + rand()) / (0.1 + RAND_MAX);
            t = sample_k(tm, pse, psd, psv, e, f, g, r, d, v);
            if (t == 0){
                fprintf(stderr, "sample failed\n");
                return -1;
            }
            add_on_tp(tm, d, v, t);
            tm->tokens[i][2] = t;
            e  += update_e(tm, pse, t, -1);
            f  += update_f(tm, psd, d, t, -1);
            i = tm->tokens[i][3];
        }
    }
    free(pse); pse = NULL;
    free(psd); psd = NULL;
    free(psv); psv = NULL;
    return 0;
}

TM * tm_create(int argc, char * argv[]){
    TM * tm = (TM*)calloc(1, sizeof(TM));
    tm->tmc = init_config();
    tm->tmc->set(tm->tmc, argc, argv);
    return tm;
}



