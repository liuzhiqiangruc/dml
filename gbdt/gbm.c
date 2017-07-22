/* ========================================================
 *   Copyright (C) 2017 All rights reserved.
 *
 *   filename : gbm.c
 *   author   : ***
 *   date     : 2017-07-19
 *   info     :
 * ======================================================== */
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include "gbm.h"

struct _gbm {
    DTD * train_ds;      // train dataset
    DTD * test_ds;       // test dataset
    int      k;          // class count of dataset
    int      currentsl;  // current max tree length
    int    * tree_size;  // tree size for model k
    double * f;          // pred value for train
    double * t;          // pred value for test
    DTree ** dts;        // dtrees
    GBMP p;              // gbm config
    G g_fn;              // grad function
    H h_fn;              // hessian function
    R r_fn;              // report function
};

static void init_k(GBM * gbm){
    DTD * ds = gbm->train_ds;
    int k, i;
    k = 0;
    for (i = 0; i < ds->row; i++) if (ds->y[i] > k) {
        k = ds->y[i];
    }
    gbm->k = k + 1;
}

static void eval_test(GBM * gbm, int k){
    int i, offs, l = gbm->test_ds->row;
    double *t = (double *)calloc(l, sizeof(double));
    eval_tree(gbm->test_ds, gbm->dts[k * gbm->p.max_trees + gbm->tree_size[k] - 1], t, l);
    offs = k * l;
    for (i = 0; i < l; i++){
        gbm->t[offs + i] -= t[i] * gbm->p.rate;
    }
    free(t);
    t = NULL;
}

GBM * gbm_create(G g_fn, H h_fn, R r_fn, GBMP p){
    GBM * gbm = (GBM*)calloc(1, sizeof(GBM));
    if (!gbm){
        goto gb_failed;
    }
    gbm->g_fn = g_fn;
    gbm->h_fn = h_fn;
    gbm->r_fn = r_fn;
    gbm->p = p;
    // load dataset
    DTD *(*tds)[2] = load_data(p.train_input, p.test_input, p.binary);
    if (!tds){
        goto ds_failed;
    }
    gbm->train_ds = (*tds)[0];
    gbm->test_ds  = (*tds)[1];
    init_k(gbm);
    // tree spaces
    gbm->tree_size = (int*)calloc(gbm->k, sizeof(int));
    gbm->dts = (DTree **)calloc(p.max_trees * gbm->k, sizeof(void *));
    if (!gbm->dts){
        goto dts_failed;
    }
    gbm->f = (double*)calloc(gbm->train_ds->row * gbm->k, sizeof(double));
    if (!gbm->f){
        goto train_y_failed;
    }
    if (!gbm->test_ds){
        goto ret_no_test;
    }
    gbm->t = (double*)calloc(gbm->test_ds->row * gbm->k, sizeof(double));
    if (!gbm->t){
        goto test_y_failed;
    }
ret_no_test:
    /*
    load_init(gbm);
    */
    return gbm;
test_y_failed:
    free(gbm->f);
    gbm->f = NULL;
train_y_failed:
    free(gbm->dts);
    gbm->dts = NULL;
dts_failed:
    if (gbm->train_ds) free_data(gbm->train_ds);
    if (gbm->test_ds)  free_data(gbm->test_ds);
    free(tds);
    tds = NULL;
ds_failed:
    free(gbm);
    gbm = NULL;
gb_failed:
    return NULL;
}

int    gbm_train(GBM * gbm){
    int i, j, k, m, n, offs, toffs, t;
    double *f, *g, *h, *e;
    DTree * tt = NULL;
    m = gbm->p.min_node_ins;
    n = gbm->train_ds->row;
    f = (double*)calloc(n, sizeof(double));
    h = (double*)calloc(n, sizeof(double));
    e = (double*)calloc(n * gbm->k, sizeof(double));
    for (i = 0; i < n; i++){
        h[i] = 1.0;
    }
    if (m < 1){
        m = (int)(0.5 * n / gbm->p.max_leaf_nodes);
        if (m < 1){
            m = 1;
        }
    }
    for (i = 0; i < gbm->p.max_trees; i++) {
        t = 0;
        gbm->g_fn(gbm->f, gbm->train_ds->y, e, n, gbm->k);
        for (k = 0; k < gbm->k; k++){
            offs  = k * n;
            toffs = k * gbm->p.max_trees;
            g     = e + offs;
            tt    = generate_dtree(gbm->train_ds, f, g, h,      \
                                   gbm->p.nod_reg,              \
                                   gbm->p.wei_reg, n, m,        \
                                   gbm->p.max_depth,            \
                                   gbm->p.max_leaf_nodes);
            if (tt){
                t = 1;
                gbm->dts[toffs + gbm->tree_size[k]] = tt;
                gbm->tree_size[k] += 1;
                for (j = 0; j < n; j++){
                    gbm->f[offs + j] -= f[j] * gbm->p.rate;
                }
                memset(f, 0, sizeof(double) * n);
                if (gbm->test_ds){
                    eval_test(gbm, k);
                }
            }
        }
        if (t == 1){
            gbm->r_fn(gbm);
            gbm->currentsl += 1;
        }
        else{
            break;
        }
    }
    free(f);  f = NULL;
    free(e);  e = NULL;
    free(h);  h = NULL;
    return 0;
}

void gbm_save(GBM * gbm){
    int     i, j, k, s, n, offs;
    char    subdir[256] = {0};
    char    outfp[256]  = {0};
    char  * outdir = gbm->p.out_dir;
    FILE  * fp = NULL;
    DTree * st = NULL;
    k  = gbm->k;
    s  = size_dtree(gbm->dts[0]);
    st = (DTree*)calloc(2000, s);   // tree space
    for (i = 0; i < k; i++){
        // create subdir for k boosting trees
        snprintf(subdir, 255, "%s/%d", outdir, i);
        mkdir(subdir, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
        // save dtree debug information
        offs = i * gbm->p.max_trees;
        for (j = 0; j < gbm->tree_size[i]; j++){
            snprintf(outfp, 255, "%s/%d.dat", subdir, j);
            save_dtree(gbm->dts[offs + j], outfp, gbm->train_ds->id_map);
        }
        // save binary format of dtrees for each boosting model
        snprintf(outfp, 255, "%s/gbmdtree.%05d.mdl", gbm->p.out_dir, i);
        if (NULL == (fp = fopen(outfp, "wb"))){
            return ;
        }
        fwrite(&gbm->train_ds->col, sizeof(int), 1, fp);
        fwrite(gbm->train_ds->id_map, sizeof(char[FKL]), gbm->train_ds->col, fp);
        for (j = 0; j < gbm->tree_size[i]; j++){
            n = serialize_dtree(gbm->dts[i], st);
            fwrite(&n, sizeof(int), 1, fp);
            fwrite(st, s, n, fp);
        }
        fclose(fp);
    }
    free(st); st = NULL;
    // save model scores for train dataset
    n = gbm->train_ds->row;
    snprintf(outfp, 255, "%s/train_score.scr", gbm->p.out_dir);
    if (NULL == (fp = fopen(outfp, "w"))){
        return;
    }
    for (i = 0; i < n; i++){
        for(j = 0; j < k; j++){
            fprintf(fp, "\t%.5f", gbm->f[j * n + i]);
        }
        fprintf(fp, "\n");
    }
    fclose(fp);
    if (gbm->test_ds){ //save scores for test dataset if need
        n = gbm->test_ds->row;
        snprintf(outfp, 255, "%s/test_score.scr", gbm->p.out_dir);
        if (NULL == (fp = fopen(outfp, "w"))){
            return;
        }
        for (i = 0; i < n; i++){
            for(j = 0; j < k; j++){
                fprintf(fp, "\t%.5f", gbm->t[j * n + i]);
            }
            fprintf(fp, "\n");
        }
        fclose(fp);
    }
    return;
}

void   gbm_free (GBM * gbm){
    int i, j, k, offs;
    k = gbm->k;
    if (gbm){
        if (gbm->train_ds){
            free_data(gbm->train_ds);
            gbm->train_ds = NULL;
        }
        if (gbm->test_ds){
            free(gbm->test_ds);
            gbm->test_ds = NULL;
        }
        if (gbm->f){
            free(gbm->f);
            gbm->f = NULL;
        }
        if (gbm->t){
            free(gbm->t);
            gbm->t = NULL;
        }
        if (gbm->dts){
            for (i = 0; i < k; i++){
                offs = i * gbm->p.max_trees;
                for (j = 0; j < gbm->tree_size[i]; j++){
                    if (gbm->dts[offs + j]){
                        free_dtree(gbm->dts[offs + j]);
                        gbm->dts[offs + j] = NULL;
                    }
                }
            }
            free(gbm->dts);
            gbm->dts = NULL;
        }
        if (gbm->tree_size){
            free(gbm->tree_size);
            gbm->tree_size = NULL;
        }
        free(gbm);
    }
}

int k_count(GBM * gbm){
    return gbm->k;
}

int y_rowns(GBM * gbm){
    return gbm->train_ds->row;
}

int t_rowns(GBM * gbm){
    return gbm->test_ds->row;
}

double * y_model(GBM * gbm){
    return gbm->f;
}

double * t_model(GBM * gbm){
    return gbm->t;
}

double * y_label(GBM * gbm){
    return gbm->train_ds->y;
}

double * t_label(GBM * gbm){
    return gbm->test_ds->y;
}

int t_size(GBM * gbm){
    return gbm->currentsl;
}

int has_test(GBM * gbm){
    return (gbm->test_ds ? 1 : 0);
}
