/* ========================================================
 *   Copyright (C) 2016 All rights reserved.
 *   
 *   filename : d2v.c
 *   author   : ***
 *   date     : 2016-12-15
 *   info     : 
 * ======================================================== */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "hash.h"
#include "str.h"
#include "repo.h"
#include "d2v.h"


D2V * d2v_create(int argc, char * argv[]){
    D2V * d2v = (D2V*)calloc(1, sizeof(D2V));
    d2v->dc = init_d_config();
    if (0 != d2v->dc->set(d2v->dc, argc, argv)){
        d2v->dc->help();
        free(d2v);
        return NULL;
    }
    return d2v;
}


static Hash * d2v_load_v(D2V * d2v){
    char * outdir = d2v->dc->get_o(d2v->dc);
    char out[512] = {0};
    char buf[10000] = {0};
    char *string;
    FILE *fp;
    
    Hash * vhs = hash_create(1 << 20, STRING);

    sprintf(out, "%s/vector", outdir);

    if (NULL == (fp = fopen(out, "r"))){
        hash_free(vhs);
        vhs = NULL;
        goto ret;
    }
    while (NULL != fgets(buf, 10000, fp)){
        string = trim(buf, 3);
        hash_add(vhs, strsep(&string, "\t"));
    }
    fclose(fp);
    
ret:
    return vhs;
}

static void d2v_load_model(D2V * d2v){
    char * outdir = d2v->dc->get_o(d2v->dc);
    char out[512] = {0};
    FILE * fp;
    int i = 0;
    sprintf(out, "%s/dvector", outdir);
    if (NULL == (fp = fopen(out, "r"))){
        return;
    }
    fscanf(fp, "%lf", d2v->u);
    while (!feof(fp)){
        i += 1;
        fscanf(fp, "%lf", d2v->u + i);
    }
    fclose(fp);
}

static void d2v_weight_init(D2V * d2v){
    int d, k, i, t;
    d = d2v->ds->d;
    k = d2v->dc->get_k(d2v->dc);
    t = d2v->dc->get_t(d2v->dc);

    d2v->u = (double*)calloc(d * k, sizeof(double));

    i = d * k;
    while (i-- > 0){
        d2v->u[i] = ((rand() + 0.1) / (RAND_MAX + 0.1) - 0.5) / k;
    }

    if (t > 0){
        d2v_load_model(d2v);
    }
}

int d2v_init(D2V * d2v){
    int v, k, t;
    char *outdir;

    t = d2v->dc->get_t(d2v->dc);
    k = d2v->dc->get_k(d2v->dc);

    if (t > 0){
        outdir = d2v->dc->get_o(d2v->dc);
        if (0 != hsoft_load(&(d2v->hsf), outdir, k)){
            return -1;
        }
        Hash * vhs = d2v_load_v(d2v);

        if (vhs){
            d2v->ds = tsd_load_v(d2v->dc->get_d(d2v->dc), vhs);
            hash_free(vhs);
            vhs = NULL;
            goto initd;
        }
        return -1;
    }

    d2v->ds   = tsd_load(d2v->dc->get_d(d2v->dc));
    v = d2v->ds->v;

    hsoft_build (&(d2v->hsf), d2v->ds->fcnt, v, k);

initd:
    d2v_weight_init(d2v);

    return 0;

}

void d2v_learn(D2V * d2v){
    int d, id, ds, de, m, tid, k, n, t;
    double alpha;
    double *cw = NULL, *eu = NULL;
    k      = d2v->dc->get_k(d2v->dc);
    n      = d2v->dc->get_n(d2v->dc);
    t      = d2v->dc->get_t(d2v->dc);
    alpha  = d2v->dc->get_alpha(d2v->dc);
    eu = (double*)malloc(k * sizeof(double));
    while (n-- > 0) for (d = 0; d < d2v->ds->d; d++){
        cw = d2v->u + d * k;
        ds = d2v->ds->doffs[d];
        de = d2v->ds->doffs[d + 1];
        for (id = ds; id < de; id++){
            tid = d2v->ds->tokens[id];
            memset(eu, 0, k * sizeof(double));

            if (t == 2){ // h fix
                hsoft_learn(d2v->hsf, cw, eu, tid, 0.0);
            }
            else {
                hsoft_learn(d2v->hsf, cw, eu, tid, alpha);
            }

            // doc fix
            if (t != 3) for (m = 0; m < k; m++){
                cw[m] += alpha * eu[m];
            }
        }
        progress(stderr, d2v->ds->d, d + 1, 0.0,0.0);
    }
    free(eu); eu = NULL;
}

void d2v_save(D2V * d2v){
    char * outdir = d2v->dc->get_o(d2v->dc);
    char out[512] = {0};
    FILE * fp = NULL;
    int i, k, t;
    k = d2v->dc->get_k(d2v->dc);
    hsoft_save(d2v->hsf, outdir);
    sprintf(out, "%s/dvector", outdir);
    if (NULL == (fp = fopen(out, "w"))){
        return;
    }
    for (i = 0; i < d2v->ds->d; i++){
        fprintf(fp, "%.3f", d2v->u[i * k]);
        for (t = 1; t < k; t++){
            fprintf(fp, "\t%.3f", d2v->u[i * k + t]);
        }
        fprintf(fp, "\n");
    }
    fclose(fp);
}

void d2v_free(D2V * d2v){
    if (d2v->hsf){
        hsoft_free(d2v->hsf);
        d2v->hsf = NULL;
    }
    if (d2v->dc){
        d2v->dc->free(d2v->dc);
        d2v->dc = NULL;
    }
    if (d2v->ds){
        tsd_free(d2v->ds);
        d2v->ds = NULL;
    }
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
