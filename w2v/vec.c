/* ========================================================
 *   Copyright (C) 2016 All rights reserved.
 *   
 *   filename : vec.c
 *   author   : liuzhiqiangruc@126.com
 *   date     : 2016-10-27
 *   info     : 
 * ======================================================== */
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "str.h"
#include "vec.h"

#define MTDEPT 40
#define MLEN   8192

static int vec_cmp(const void *a, const void *b){
    int c = ((int*)a)[1] - ((int*)b)[1];
    return c > 0 ? -1 : (c < 0 ? 1 : 0);
}

void vec_build_tree(Vec * vec, int (*wc)[2], int n){
    int p, ip, ipi, t = 0;
    int * inner_cnt = (int*)calloc(n, sizeof(int));
    qsort(wc, n, sizeof(int[2]), vec_cmp);
    p = n - 1;
    ip = ipi = 0;
    while (p >= 0 || ip < ipi){
        if (p >= 0){
            if (ip < ipi && wc[p][1] > inner_cnt[ip]){
                goto inner_link;
            }
            inner_cnt[ipi] += wc[p][1];
            vec->hbt[p][0] = ipi;
            if (t){
                vec->hbt[p][1] = 1;
                ipi += 1;
            }
            p -= 1;
        }
        else{
inner_link:
            inner_cnt[ipi] += inner_cnt[ip];
            vec->hbt[ip][2] = ipi;
            if (t){
                vec->hbt[ip][3] = 1;
                ipi += 1;
            }
            ip += 1;
        }
        t ^= 1;
    }
    vec->hbt[ipi - 1][2] = -1; // root has no parent
    free(inner_cnt); inner_cnt = NULL;
    for (t = 0; t < n; t++){
        vec->hbt[wc[t][0]][4] = t;
    }
}

void vec_learn_tree(Vec * vec, float * cw, float * de, int vid, double learn_rate){
    int s = 0, t = 0, k = vec->k, st[MTDEPT], sb[MTDEPT];
    double loss = 0.0;
    memset(st, -1, sizeof(int) * MTDEPT);
    memset(sb, -1, sizeof(int) * MTDEPT);
    st[t] = vec->hbt[vec->hbt[vid][4]][0];
    sb[t] = vec->hbt[vec->hbt[vid][4]][1];
    while (-1 != st[t]){
        s = t;
        t = (t + 1) % MTDEPT;
        st[t] = vec->hbt[st[s]][2];
        sb[t] = vec->hbt[st[s]][3];
    }
    while (-1 != st[s]){
        loss = 0.0;
        for (t = 0; t < k; t++){
            loss += cw[t] * vec->neu1[st[s] * k + t];
        }
        loss = learn_rate * (sb[s] - 1.0 / (1.0 + exp(-loss)));
        if (fabs(loss) > 1e-9) for (t = 0; t < k; t++){
            de[t] += loss * vec->neu1[st[s] * k + t];
            if (vec->t == 0){
                vec->neu1[st[s] * k + t] += loss * cw[t];
            }
        }
        s -= 1;
        s = s < 0 ? (s + MTDEPT) : s;
    }
}

void vec_save_tree(Vec * vec, TSD * ds, char * outdir){
    char out[512] = {0};
    int i, t, v = ds->v, k = vec->k;
    FILE * fp = NULL;
    sprintf(out, "%s/noleaf", outdir);
    if (NULL == (fp = fopen(out, "w"))){
        return;
    }
    for (i = 0; i < v; i++){
        fprintf(fp, "%.3f", vec->neu1[i * k]);
        for (t = 1; t < k; t++){
            fprintf(fp, "\t%.3f", vec->neu1[i * k + t]);
        }
        fprintf(fp, "\n");
    }
    fclose(fp);
    sprintf(out, "%s/index", outdir);
    if (NULL == (fp = fopen(out, "w"))){
        return;
    }
    for (i = 0; i < v; i++){
        fprintf(fp, "%s\t%d\t%d\t%d\t%d\n"          \
                  , ds->idm[i]                      \
                  , vec->hbt[vec->hbt[i][4]][0]     \
                  , vec->hbt[vec->hbt[i][4]][1]     \
                  , vec->hbt[i][2], vec->hbt[i][3]);
    }
    fclose(fp);
}

void vec_load_tree(Vec * vec, TSD * ds, char * outdir, char * leaff){
    int v = ds->v, i;
    char *string = NULL, *token = NULL;
    char out[512] = {0};
    char buf[MLEN] = {0};
    FILE * fp = NULL;
    sprintf(out, "%s/index", outdir);
    if (NULL == (fp = fopen(out, "r"))){
        return ;
    }
    if (v == 0){
        while (NULL != fgets(buf, MLEN, fp)){
            v += 1;
        }
        ds->v = v;
        rewind(fp);
    }
    if (!vec->hbt){
        vec->hbt = (int(*)[5])calloc(v, sizeof(int[5]));
    }
    if (!ds->idm){
        ds->idm = (char(*)[KEY_SIZE])calloc(v, sizeof(char[KEY_SIZE]));
    }
    i = 0;
    while (NULL != fgets(buf, MLEN, fp)){
        string = trim(buf, 3);
        token = strsep(&string, "\t");
        strncpy(ds->idm[i], token, KEY_SIZE - 1);
        vec->hbt[i][0] = atoi(strsep(&string, "\t"));
        vec->hbt[i][1] = atoi(strsep(&string, "\t"));
        vec->hbt[i][2] = atoi(strsep(&string, "\t"));
        vec->hbt[i][3] = atoi(strsep(&string, "\t"));
        vec->hbt[i][4] = i;
        i += 1;
    }
    fclose(fp);
    sprintf(out, "%s/noleaf", outdir);
    if (NULL == (fp = fopen(out, "r"))){
        return;
    }
    if (!vec->neu1){
        vec->neu1 = (float*)calloc(v * vec->k, sizeof(float));
    }
    i = 0;
    while (NULL != fgets(buf, MLEN, fp)){
        string = trim(buf, 3);
        while (NULL != (token = strsep(&string, "\t"))){
            vec->neu1[i++] = atof(token);
        }
    }
    fclose(fp);
    sprintf(out, "%s/%s", outdir, leaff);
    if (NULL == (fp = fopen(out, "r"))){
        return;
    }
    v = 0;
    while (NULL != fgets(buf, MLEN, fp)){
        v += 1;
    }
    rewind(fp);
    if (!vec->neu1){
        vec->neu0 = (float*)calloc(v * vec->k, sizeof(float));
    }
    i = 0;
    while (NULL != fgets(buf, MLEN, fp)){
        string = trim(buf, 3);
        while (NULL != (token = strsep(&string, "\t"))){
            vec->neu0[i++] = atof(token);
        }
    }
    fclose(fp);
}

void vec_free_tree(Vec * vec){
    free(vec->hbt);
    free(vec->neu0);
    free(vec->neu1);
    vec->hbt = NULL;
    vec->neu0 = vec->neu1 = NULL;
}
