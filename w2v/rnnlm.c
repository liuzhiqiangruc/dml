/* ========================================================
 *   Copyright (C) 2016 All rights reserved.
 *   
 *   filename : rnnlm.c
 *   author   : ***
 *   date     : 2016-12-07
 *   info     : rnnlm implementation usgin sgd per token
 * ======================================================== */

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "rnnlm.h"

static void rnn_weight_init(RNNLM * rnnlm){
    int i, l, v, k;

    v = rnnlm->ds->v;
    k = rnnlm->rc->get_k(rnnlm->rc);
    l = rnnlm->rc->get_w(rnnlm->rc);

    rnnlm->rnn->u = calloc(v * k, sizeof(double));
    rnnlm->rnn->w = calloc(k * k, sizeof(double));
    rnnlm->rnn->s = calloc(l * k, sizeof(double));

    i = v * k;
    while (i-- > 0){
        rnnlm->rnn->u[i] = ((rand() + 0.1) / (RAND_MAX + 0.1) - 0.5) / v;
    }

    i = k * k;
    while (i-- > 0){
        rnnlm->rnn->w[i] = ((rand() + 0.1) / (RAND_MAX + 0.1) - 0.5) / k;
    }
}

RNNLM * rnn_create(int argc, char *argv[]){
    RNNLM * rnnlm = (RNNLM*)calloc(1, sizeof(RNNLM));

    rnnlm->rc = init_rnn_config();

    if (0 != rnnlm->rc->set(rnnlm->rc, argc, argv)){
        rnnlm->rc->help();
        free(rnnlm);
        rnnlm = NULL;
    }

    return rnnlm;
}

int rnn_init  (RNNLM * rnnlm){
    int v, k;

    // load train data
    rnnlm->ds     = tsd_load(rnnlm->rc->get_d(rnnlm->rc));
    v             = rnnlm->ds->v;
    k             = rnnlm->rc->get_k(rnnlm->rc);

    // consturct the hierarchical softmax tree
    hsoft_build(&(rnnlm->hsf), rnnlm->ds->fcnt, v, k);

    // init the rnnlm parameters
    rnn_weight_init(rnnlm);

    return 0;
}

static inline double * forward_st (RNNLM * rnnlm, int id, int sid, int w, int k){

    double *in, *ls, *st;
    int i, j, tid, lsid;

    tid = rnnlm->ds->tokens[id];
    st = rnnlm->rnn->s + (sid % w) * k;
    in = rnnlm->rnn->u + tid * k;

    memcpy(st, in, sizeof(double) * k);
    if (sid > 0){
        lsid = (sid - 1) % w;
        ls = rnnlm->rnn->s + lsid  * k;
    }

    for (i = 0; i < k; i++){
        if (sid > 0) for (j = 0; j < k; j++){
            st[i] += rnnlm->rnn->w[i * k + j] * ls[j];
        }
        st[i] = 1.0 / (1.0 + exp(-st[i]));
    }

    return st;
}

static inline void accumulate_grad_w (RNNLM * rnnlm, double * sg, double * wg, int lsid, int k){
    double *ls;
    int i, j;

    ls = rnnlm->rnn->s + lsid * k;

    for (i = 0; i < k; i++){
        for (j = 0; j < k; j++){
            wg[i * k + j] += sg[i] * ls[j];
        }
    }
}

static inline void update_input_u (RNNLM * rnnlm, double * sg, double alpha, int id, int k){
    int tid, i;
    double * in;

    tid = rnnlm->ds->tokens[id];
    in  = rnnlm->rnn->u + tid * k;

    for (i = 0; i < k; i++){
        in[i] += alpha * sg[i];
    }
}

static inline void back_propgation_s (RNNLM * rnnlm, double * sg, double *tg, int lsid, int k) {
    int i, j;
    double *ls;
    ls = rnnlm->rnn->s + lsid * k;

    memset(tg, 0, sizeof(double) * k);

    for (i = 0; i < k; i++){
        for (j = 0; j < k; j++){
            tg[i] += rnnlm->rnn->w[j * k + i] * sg[j];
        }
        tg[i] *= ls[i] * (1.0 - ls[i]);
    }

    memcpy(sg, tg, sizeof(double) * k);
}

static inline void update_w (RNNLM * rnnlm, double * wg, int k, double alpha){
    int i, j;
    for (i = 0; i < k; i++){
        for(j = 0; j < k; j++){
            rnnlm->rnn->w[i * k + j] += alpha * wg[i * k + j];
        }
    }
}


void rnn_learn(RNNLM * rnnlm){
    double *st, *sg, *tg, *wg, alpha;
    int i, k, n, w, d, id, ds, de, sid, lsid;

    n     = rnnlm->rc->get_n(rnnlm->rc);
    k     = rnnlm->rc->get_k(rnnlm->rc);
    w     = rnnlm->rc->get_w(rnnlm->rc);
    alpha = rnnlm->rc->get_alpha(rnnlm->rc);
    tg    = (double*)malloc(sizeof(double) * k);
    sg    = (double*)malloc(sizeof(double) * k);
    wg    = (double*)malloc(sizeof(double) * k * k);

    while (n-- > 0) for (d = 0; d < rnnlm->ds->d; d++){
        sid = 0;   // first token of current doc | sentence
        ds = rnnlm->ds->doffs[d];         // begin pos
        de = rnnlm->ds->doffs[d + 1];     // end   pos
        memset(wg, 0, sizeof(double) * k * k);
        if (de - ds > 1) for (id = ds; id < de - 1; id++){
            st = forward_st (rnnlm, id, sid, w, k);

            memset(sg, 0, sizeof(double) * k);
            hsoft_learn(rnnlm->hsf, st, sg, rnnlm->ds->tokens[id + 1], alpha);
            for (i = 0; i < k; i++){
                sg[i] *= st[i] * (1.0 - st[i]);
            }
            update_input_u (rnnlm, sg, alpha, id, k);

            for (i = 1; i < w; i++) if ((lsid = sid - i) >= 0) {
                lsid = lsid % w;
                accumulate_grad_w (rnnlm, sg, wg, lsid, k);
                back_propgation_s (rnnlm, sg, tg, lsid, k);
                update_input_u    (rnnlm, sg, alpha, id - i, k);
            }
            sid += 1;
        }
        if (de - ds > 1) {
            update_w(rnnlm, wg, k, alpha);
        }
    }

    free(sg);      sg = NULL;
    free(tg);      tg = NULL;
    free(wg);      wg = NULL;
}
