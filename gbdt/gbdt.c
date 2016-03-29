/* ========================================================
 *   Copyright (C) 2016 All rights reserved.
 *   
 *   filename : gbdt.c
 *   author   : liuzhiqiangruc@126.com
 *   date     : 2016-02-26
 *   info     : implementation for gbdt
 * ======================================================== */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "gbdt.h"

struct _gbdt {
    DTD * train_ds;
    DTD *  test_ds;
    double * f;
    double * t;
    int tree_size;
    DTree ** dts;
    GBMP p;
    G g_fn;
    H h_fn;
    R r_fn;
};

GBDT * gbdt_create(G g_fn, H h_fn, R r_fn, GBMP p){
    GBDT * gbdt = (GBDT*)malloc(sizeof(GBDT));
    if (!gbdt){
        goto gb_failed;
    }
    memset(gbdt, 0, sizeof(GBDT));
    gbdt->g_fn = g_fn;
    gbdt->h_fn = h_fn;
    gbdt->r_fn = r_fn;
    gbdt->p = p;
    gbdt->dts = (DTree **)malloc(sizeof(void *) * p.max_trees);
    if (!gbdt->dts){
        goto dts_failed;
    }
    memset(gbdt->dts, 0, sizeof(void *) * p.max_trees);
    DTD *(*tds)[2] = load_data(p.train_input, p.test_input, p.binary);
    if (!tds){
        goto ds_failed;
    }
    gbdt->train_ds = (*tds)[0];
    gbdt->test_ds  = (*tds)[1];
    gbdt->f = (double*)malloc(sizeof(double) * gbdt->train_ds->row);
    if (!gbdt->f){
        goto train_y_failed;
    }
    memset(gbdt->f, 0, sizeof(double) * gbdt->train_ds->row);
    gbdt->t = (double*)malloc(sizeof(double) * gbdt->test_ds->row);
    if (!gbdt->t){
        goto test_y_failed;
    }
    memset(gbdt->t, 0, sizeof(double) * gbdt->test_ds->row);

    return gbdt;

test_y_failed:
    free(gbdt->f);
    gbdt->f = NULL;
train_y_failed:
    free_data(gbdt->train_ds);
    free_data(gbdt->test_ds);
    gbdt->train_ds = NULL;
    gbdt->test_ds  = NULL;
ds_failed:
    free(gbdt->dts);
    gbdt->dts = NULL;
dts_failed:
    free(gbdt);
    gbdt = NULL;
gb_failed:
    return NULL;
}

int    gbdt_train(GBDT * gbdt){
    int i, j, n = gbdt->train_ds->row, l = gbdt->test_ds->row;
    double * f = (double*)malloc(sizeof(double) * n);
    double * t = (double*)malloc(sizeof(double) * l);
    double * g = (double*)malloc(sizeof(double) * n);
    double * h = (double*)malloc(sizeof(double) * n);
    memset(f, 0, sizeof(double) * n);
    memset(t, 0, sizeof(double) * l);
    memset(g, 0, sizeof(double) * n);
    memset(h, 0, sizeof(double) * n);
    gbdt->tree_size = 0;

    for (i = 0; i < gbdt->p.max_trees; i++) {
        gbdt->g_fn(gbdt->f, gbdt->train_ds->y, g, n);
        gbdt->h_fn(gbdt->f, gbdt->train_ds->y, h, n);

        DTree * tt = generate_dtree(gbdt->train_ds, f, g, h, n, gbdt->p.max_leaf_nodes);
        if (tt){
            gbdt->dts[i] = tt;
            gbdt->tree_size += 1;
            eval_tree(gbdt->test_ds, tt, t, l);
            for (j = 0; j < n; j++){
                gbdt->f[j] += f[j] * gbdt->p.rate;
            }
            for (j = 0; j < l; j++){
                gbdt->t[j] += t[j] * gbdt->p.rate;
            }
            memset(f, 0, sizeof(double) * n);
            memset(t, 0, sizeof(double) * l);
            gbdt->r_fn(gbdt);
        }
        else{
            break;
        }
    }
    free(f);  f = NULL;
    free(t);  t = NULL;
    free(g);  g = NULL;
    free(h);  h = NULL;

    return 0;
}

void   gbdt_save (GBDT * gbdt){
    int i;
    char outfile[200] = {'\0'};
    for (i = 0; i < gbdt->tree_size; i++){
        snprintf(outfile, 200, "%s/%d.dat", gbdt->p.out_dir, i);
        save_dtree(gbdt->dts[i], outfile, gbdt->train_ds->id_map);
    }
}

void   gbdt_free (GBDT * gbdt){
    int i;
    if (gbdt){
        if (gbdt->train_ds){
            free_data(gbdt->train_ds);
            gbdt->train_ds = NULL;
        }
        if (gbdt->test_ds){
            free(gbdt->test_ds);
            gbdt->test_ds = NULL;
        }
        if (gbdt->f){
            free(gbdt->f);
            gbdt->f = NULL;
        }
        if (gbdt->t){
            free(gbdt->t);
            gbdt->t = NULL;
        }
        if (gbdt->dts){
            for (i = 0; i < gbdt->tree_size; i++){
                if (gbdt->dts[i]){
                    free_dtree(gbdt->dts[i]);
                    gbdt->dts[i] = NULL;
                }
            }
            free(gbdt->dts);
            gbdt->dts = NULL;
        }
        free(gbdt);
    }
}

int y_rowns(GBDT * gbdt){
    return gbdt->train_ds->row;
}

int t_rowns(GBDT * gbdt){
    return gbdt->test_ds->row;
}

double * y_model(GBDT * gbdt){
    return gbdt->f;
}

double * t_model(GBDT * gbdt){
    return gbdt->t;
}

double * y_label(GBDT * gbdt){
    return gbdt->train_ds->y;
}

double * t_label(GBDT * gbdt){
    return gbdt->test_ds->y;
}

int t_size(GBDT * gbdt){
    return gbdt->tree_size;
}
