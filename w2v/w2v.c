/* ========================================================
 *   Copyright (C) 2016 All rights reserved.
 *   
 *   filename : w2v.c
 *   author   : ***
 *   date     : 2016-12-14
 *   info     : 
 * ======================================================== */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "hash.h"
#include "str.h"
#include "repo.h"
#include "w2v.h"

static inline void w2v_st(W2V * w2v, double *st, int id, int k, int l, int r){
    int  c, i, j, vi;
    double *u;

    c = 0;

    for (i = l; i < r; i++) if (i != id){
        vi = w2v->ds->tokens[i];
        u  = w2v->u + vi * k;
        for (j = 0; j < k; j++){
            st[j] += u[j];
        }
        c += 1;
    }

    if (c > 1) for (j = 0; j < k; j++){
        st[j] /= c;
    }
}

static inline void w2v_ut(W2V * w2v, double * sg, int id, int k, int l, int r, double alpha){
    int i, j, vi;
    double * u;
    for (i = l; i < r; i++) if (i != id){
        vi = w2v->ds->tokens[i];
        u  = w2v->u + vi * k;
        for (j = 0; j < k; j++){
            u[j] += alpha * sg[j];
        }
    }
}

static Hash * w2v_load_model(W2V * w2v){
    char * outdir = w2v->wc->get_o(w2v->wc);
    char out[512] = {0};
    char buffer[10000] = {0};
    char *string, *token;
    FILE *fp = NULL;

    int v, k, i;
    Hash * vhs = hash_create(STRING, 1 << 20);
    v = w2v->hsf->v;
    k = w2v->wc->get_k(w2v->wc);
    sprintf(out, "%s/vector", outdir);
    if (NULL == (fp = fopen(out, "r"))){
        hash_free(vhs);
        vhs = NULL;
        goto ret;
    }
    i = 0;
    while (NULL != fgets(buffer, 10000, fp)){
        string = trim(buffer, 3);
        token = strsep(&string, "\t");
        hash_add(vhs, token);
        while (NULL != (token = strsep(&string, "\t"))){
            w2v->u[i++] = atof(token);
        }
    }
    fclose(fp);

ret:
    return vhs;
}

static Hash * w2v_weight_init(W2V * w2v){
    int i, l, v, k, t;
    v = w2v->hsf->v;
    k = w2v->wc->get_k(w2v->wc);
    t = w2v->wc->get_t(w2v->wc);

    w2v->u = (double *)calloc(v * k, sizeof(double));

    if (t == 0){
        i = v * k;
        while (i-- > 0){
            w2v->u[i] = ((rand() + 0.1) / (RAND_MAX + 0.1) - 0.5) / v;
        }
        return NULL;
    }
    else {
        return w2v_load_model(w2v);
    }
}

W2V * w2v_create(int argc, char *argv[]){
    W2V * w2v = (W2V*)calloc(1, sizeof(W2V));

    w2v->wc = init_w2v_config();

    if (0 != w2v->wc->set(w2v->wc, argc, argv)){
        w2v->wc->help();
        free(w2v);
        w2v = NULL;
    }

    return w2v;
}

int w2v_init(W2V * w2v){
    int v, k, t;
    char *outdir;

    t = w2v->wc->get_t(w2v->wc);
    k = w2v->wc->get_k(w2v->wc);

    if (t == 1){
        outdir = w2v->wc->get_o(w2v->wc);
        if (0 != hsoft_load(&(w2v->hsf), outdir, k)){
            return -1;
        }
        Hash * vhs = w2v_weight_init(w2v);

        if (vhs){
            w2v->ds = tsd_load_v(w2v->wc->get_d(w2v->wc), vhs);
            hash_free(vhs);
            vhs = NULL;
            return 0;
        }
        return -1;
    }

    w2v->ds   = tsd_load(w2v->wc->get_d(w2v->wc));
    v = w2v->ds->v;

    hsoft_build (&(w2v->hsf), w2v->ds->fcnt, v, k);

    w2v_weight_init(w2v);

    return 0;
}

void w2v_learn (W2V * w2v){
    int i, k, n, w, d, id, ds, de, l, r;
    double *st, *sg;
    double alpha;

    w     = w2v->wc->get_w(w2v->wc);
    n     = w2v->wc->get_n(w2v->wc);
    k     = w2v->wc->get_k(w2v->wc);
    alpha = w2v->wc->get_alpha(w2v->wc);

    st = (double *)calloc(k, sizeof(double));
    sg = (double *)calloc(k, sizeof(double));

    while (n-- > 0) for (d = 0; d < w2v->ds->d; d++){
        ds = w2v->ds->doffs[d];
        de = w2v->ds->doffs[d + 1];

        if (de - ds > 1) for (id = ds; id < de; id ++){
            l = id - w / 2;
            l = l < ds ? ds : l;
            r = l + w;
            r = r > de ? de : r;

            memset(st, 0, sizeof(double) * k);
            memset(sg, 0, sizeof(double) * k);

            w2v_st(w2v, st, id, k, l, r);
            hsoft_learn(w2v->hsf, st, sg, w2v->ds->tokens[id], alpha);
            w2v_ut(w2v, sg, id, k, l, r, alpha);
        }
        progress(stderr, w2v->ds->d, d + 1);
    }

    free(st); st = NULL;
    free(sg); sg = NULL;
}

void w2v_save(W2V * w2v){
    char * outdir = w2v->wc->get_o(w2v->wc);
    char out[512];
    FILE * fp = NULL;
    int i, k, t;
    k = w2v->wc->get_k(w2v->wc);
    hsoft_save(w2v->hsf, outdir);

    sprintf(out, "%s/vector", outdir);
    if (NULL == (fp = fopen(out, "w"))){
        return;
    }
    for (i = 0; i < w2v->ds->v; i++){
        fprintf(fp, "%s", w2v->ds->idm[i]);
        for (t = 0; t < k; t++){
            fprintf(fp, "\t%.3f", w2v->u[i * k + t]);
        }
        fprintf(fp, "\n");
    }
    fclose(fp);
}

void w2v_free(W2V * w2v){
    if (w2v->ds){
        tsd_free(w2v->ds);
        w2v->ds = NULL;
    }
    if (w2v->hsf){
        hsoft_free(w2v->hsf);
        w2v->hsf = NULL;
    }
    if (w2v->u){
        free(w2v->u);
        w2v->u = NULL;
    }
    if (w2v->wc){
        w2v->wc->free(w2v->wc);
        w2v->wc = NULL;
    }
    free(w2v);
}
