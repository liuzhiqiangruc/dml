/* ========================================================
 *   Copyright (C) 2016 All rights reserved.
 *   
 *   filename : doc2vec.c
 *   author   : liuzhiqiangruc@126.com
 *   date     : 2016-10-27
 *   info     : 
 * ======================================================== */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "repo.h"
#include "doc2vec.h"

D2V * d2v_create(int argc, char * argv[]){
    D2V * d2v = (D2V*)calloc(1, sizeof(D2V));
    d2v->dc = init_d_config();
    if (0 != d2v->dc->set(d2v->dc, argc, argv)){
        d2v->dc->help();
        return NULL;
    }
    return d2v;
}

int d2v_init(D2V * d2v){
    int i, k = d2v->dc->get_k(d2v->dc), t = d2v->dc->get_t(d2v->dc);
    d2v->ds          = tsd_load(d2v->dc->get_d(d2v->dc));
    d2v->model       = (Vec*)calloc(1, sizeof(Vec));
    d2v->model->k    = k;
    d2v->model->t    = 0;
    if (t == 2){
        d2v->model->t = 1;
    }
    d2v->model->hbt  = (int(*)[5])calloc(d2v->ds->v, sizeof(int[5]));
    d2v->model->neu0 = (float*)calloc(d2v->ds->d * k, sizeof(float));
    d2v->model->neu1 = (float*)calloc(d2v->ds->v * k, sizeof(float));
    vec_build_tree(d2v->model, d2v->ds->fcnt, d2v->ds->v);
    i = d2v->ds->d * k;
    while (i-- > 0){
        d2v->model->neu0[i] = ((rand() + 0.1) / (RAND_MAX + 0.1) - 0.5) / k;
    }
    if (t > 0){
        vec_load_tree(d2v->model, d2v->ds, d2v->dc->get_o(d2v->dc), "dvector");
    }
    return 0;
}


void d2v_learn(D2V * d2v){
    int d, id, ds, de, m, tid, k, n;
    double learn_rate;
    float *cw = NULL, *eu = NULL;
    k          = d2v->dc->get_k(d2v->dc);
    n          = d2v->dc->get_n(d2v->dc);
    learn_rate = d2v->dc->get_alpha(d2v->dc);
    eu = (float*)malloc(k * sizeof(float));
    while (n-- > 0) for (d = 0; d < d2v->ds->d; d++){
        cw = d2v->model->neu0 + d * k;
        ds = d2v->ds->doffs[d];
        de = d2v->ds->doffs[d + 1];
        for (id = ds; id < de; id++){
            tid = d2v->ds->tokens[id];
            memset(eu, 0, k * sizeof(float));
            vec_learn_tree(d2v->model, cw, eu, tid, learn_rate);
            for (m = 0; m < k; m++){
                cw[m] += eu[m];
            }
        }
        progress(stderr, d2v->ds->d, d + 1);
    }
    free(eu); eu = NULL;
}

void d2v_save(D2V * d2v){
    char * outdir = d2v->dc->get_o(d2v->dc);
    char out[512] = {0};
    FILE * fp = NULL;
    int i, k, t;
    k = d2v->dc->get_k(d2v->dc);
    vec_save_tree(d2v->model, d2v->ds, outdir);
    sprintf(out, "%s/dvector", outdir);
    if (NULL == (fp = fopen(out, "w"))){
        return;
    }
    for (i = 0; i < d2v->ds->d; i++){
        fprintf(fp, "%.3f", d2v->model->neu0[i * k]);
        for (t = 1; t < k; t++){
            fprintf(fp, "\t%.3f", d2v->model->neu0[i * k + t]);
        }
        fprintf(fp, "\n");
    }
    fclose(fp);
}

void d2v_free(D2V * d2v){
    vec_free_tree(d2v->model);
    d2v->model = NULL;
    d2v->dc->free(d2v->dc);
    d2v->dc = NULL;
    tsd_free(d2v->ds);
    d2v->ds = NULL;
    free(d2v);
}

int   d2v_dsize (D2V * d2v){
    return d2v->ds->d;
}
int   d2v_vsize (D2V * d2v){
    return d2v->ds->v;
}
int   d2v_tsize (D2V * d2v){
    return d2v->ds->t;
}
