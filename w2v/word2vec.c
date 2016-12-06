/* ========================================================
 *   Copyright (C) 2016 All rights reserved.
 *   
 *   filename : word2vec.c
 *   author   : liuzhiqiangruc@126.com
 *   date     : 2016-10-27
 *   info     : 
 * ======================================================== */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "repo.h"
#include "word2vec.h"

W2V * w2v_create(int argc, char * argv[]){
    W2V * w2v = (W2V*)calloc(1, sizeof(W2V));
    w2v->wc = init_config();
    if (0 != w2v->wc->set(w2v->wc, argc, argv)){
        w2v->wc->help();
        return NULL;
    }
    return w2v;
}

int w2v_init(W2V * w2v){
    int i, k = w2v->wc->get_k(w2v->wc), t = w2v->wc->get_t(w2v->wc);
    w2v->ds          = tsd_load(w2v->wc->get_d(w2v->wc));
    w2v->model       = (Vec*)calloc(1, sizeof(Vec));
    w2v->model->k    = k;
    w2v->model->t    = 0;
    w2v->model->hbt  = (int(*)[5])calloc(w2v->ds->v, sizeof(int[5]));
    w2v->model->neu0 = (float*)calloc(w2v->ds->v * k, sizeof(float));
    w2v->model->neu1 = (float*)calloc(w2v->ds->v * k, sizeof(float));
    vec_build_tree(w2v->model, w2v->ds->fcnt, w2v->ds->v);
    i = w2v->ds->v * k;
    while (i-- > 0){
        w2v->model->neu0[i] = ((rand() + 0.1) / (RAND_MAX + 0.1) - 0.5) / k;
    }
    if (t == 1){
        vec_load_tree(w2v->model, w2v->ds, w2v->wc->get_o(w2v->wc), "vector");
    }
    return 0;
}

void w2v_learn(W2V * w2v){
    int w, d, id, ds, de, l, r, wi, m, tid, c, k, n;
    double learn_rate;
    w          = w2v->wc->get_w(w2v->wc);
    k          = w2v->wc->get_k(w2v->wc);
    n          = w2v->wc->get_n(w2v->wc);
    learn_rate = w2v->wc->get_alpha(w2v->wc);
    float * cw = (float*)malloc(k * sizeof(float));
    float * eu = (float*)malloc(k * sizeof(float));
    while (n-- > 0) for (d = 0; d < w2v->ds->d; d++){
        ds = w2v->ds->doffs[d];
        de = w2v->ds->doffs[d + 1];
        if (de - ds > 1) for (id = ds; id < de; id++){
            memset(cw, 0, sizeof(float) * k);
            if (id - w / 2 < ds) l = ds;
            else l = id - w / 2;
            r = l + w;
            if (r > de) r = de;
            c = 0;
            for (wi = l; wi < r; wi++) if (wi != id){
                tid = w2v->ds->tokens[wi];
                for (m = 0; m < k; m++){
                    cw[m] += w2v->model->neu0[tid * k + m];
                }
                c += 1;
            }
            if (c > 1) for (m = 0; m < k; m++){
                cw[m] /= c;
            }
            tid = w2v->ds->tokens[id];
            memset(eu, 0, sizeof(float) * k);
            vec_learn_tree(w2v->model, cw, eu, tid, learn_rate);
            for (wi = l; wi < r; wi++) if (wi != id){
                tid = w2v->ds->tokens[wi];
                for (m = 0; m < k; m++){
                    w2v->model->neu0[tid * k + m] += eu[m];
                }
            }
        }
        progress(stderr, w2v->ds->d, d + 1);
    }
    free(cw); cw = NULL;
    free(eu); eu = NULL;
}

void w2v_save(W2V * w2v){
    char * outdir = w2v->wc->get_o(w2v->wc);
    char out[512] = {0};
    FILE * fp = NULL;
    int i, k, t;
    k = w2v->wc->get_k(w2v->wc);
    vec_save_tree(w2v->model, w2v->ds, outdir);
    sprintf(out, "%s/vector", outdir);
    if (NULL == (fp = fopen(out, "w"))){
        return;
    }
    for (i = 0; i < w2v->ds->v; i++){
        fprintf(fp, "%.3f", w2v->model->neu0[i * k]);
        for (t = 1; t < k; t++){
            fprintf(fp, "\t%.3f", w2v->model->neu0[i * k + t]);
        }
        fprintf(fp, "\n");
    }
    fclose(fp);
}

void w2v_free(W2V * w2v){
    vec_free_tree(w2v->model);
    w2v->model = NULL;
    w2v->wc->free(w2v->wc);
    w2v->wc = NULL;
    tsd_free(w2v->ds);
    w2v->ds = NULL;
    free(w2v);
}

int   w2v_dsize (W2V * w2v){
    return w2v->ds->d;
}
int   w2v_vsize (W2V * w2v){
    return w2v->ds->v;
}
int   w2v_tsize (W2V * w2v){
    return w2v->ds->t;
}
