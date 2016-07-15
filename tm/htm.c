/* ========================================================
 *   Copyright (C) 2016 All rights reserved.
 *   
 *   filename : htm.c
 *   author   : liuzhiqiangruc@126.com
 *   date     : 2016-07-15
 *   info     : 
 * ======================================================== */

#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tm_config_h.h"
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
    unsigned int l;
    unsigned int v;
    unsigned int t;
    char (*id_d_map)[KEY_SIZE];
    char (*id_l_map)[KEY_SIZE];
    char (*id_v_map)[KEY_SIZE];
    int  (*tokens)[6];
    int  (*doc_cnt)[2];
    int   * doc_entry;
    ModelEle * nd;
    ModelEle * nw;
    int   * nkw;
    int   * wl;
    int   * ln;
    TMHConfig * tmc;
};

static void malloc_space(TM * tm){
    int k = tm->tmc->get_k(tm->tmc);
    tm->id_d_map  = (char(*)[KEY_SIZE])calloc(tm->d,   sizeof(char[KEY_SIZE]));
    tm->id_l_map  = (char(*)[KEY_SIZE])calloc(tm->l,   sizeof(char[KEY_SIZE]));
    tm->id_v_map  = (char(*)[KEY_SIZE])calloc(tm->v,   sizeof(char[KEY_SIZE]));
    tm->tokens    = (int(*)[6])        calloc(tm->t,   sizeof(int[6]));
    tm->doc_cnt   = (int(*)[2])        calloc(tm->d,   sizeof(int[2]));
    tm->doc_entry = (int *)            malloc(tm->d  * sizeof(int));
    tm->ln        = (int *)            calloc(tm->l,   sizeof(int));
    tm->nkw       = (int *)            calloc(k + 1,   sizeof(int));
    tm->wl        = (int *)            calloc(tm->v * tm->l,   sizeof(int));
    tm->nd        = (ModelEle *)       calloc(tm->d * (k + 1), sizeof(ModelEle));
    tm->nw        = (ModelEle *)       calloc(tm->v * (k + 1), sizeof(ModelEle));
    memset(tm->doc_entry, -1, tm->d * sizeof(int));
}

static void fullfill_param(TM * tm){
    int i, d, l, v, x, t, o, p, k;
    k = tm->tmc->get_k(tm->tmc);
    for (i = 0; i < tm->t; i++){
        d = tm->tokens[i][0];
        l = tm->tokens[i][1];
        v = tm->tokens[i][2];
        x = tm->tokens[i][3];
        t = tm->tokens[i][4];
        if (x == 0){
            tm->nd[d * (k + 1) + t].count += 1;
            tm->nw[v * (k + 1) + t].count += 1;
            tm->nkw[t] += 1;
            tm->doc_cnt[d][0] += 1;
        }
        else{
            tm->wl[v * tm->l + l] += 1;
            tm->ln[l]             += 1;
            tm->doc_cnt[d][1]     += 1;
        }
    }
    for (d = 0; d < tm->d; d++){
        o = d * (k + 1);
        p = 0;
        for (t = 1; t < k + 1; t++){
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
        for (t = 1; t < k + 1; t++){
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


