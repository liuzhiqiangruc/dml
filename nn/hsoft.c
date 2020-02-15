/* ========================================================
 *   Copyright (C) 2016 All rights reserved.
 *   
 *   filename : hsoft.c
 *   author   : ***
 *   date     : 2016-12-06
 *   info     : 
 * ======================================================== */

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "hsoft.h"
#include "str.h"

#define MXDEPT 40

static int int_reverse_cmp(const void *a, const void *b){
    int c = ((int*)a)[1] - ((int*)b)[1];
    return c > 0 ? -1 : (c < 0 ? 1 : 0);
}

static void hsoft_build_tree(int (*hbt)[5], int (*wc)[2], int n){
    int p, ip, ipi, t = 0;
    int * inner_cnt = (int*)calloc(n, sizeof(int));
    qsort(wc, n, sizeof(int[2]), int_reverse_cmp);
    p = n - 1;
    ip = ipi = 0;
    while (p >= 0 || ip < ipi){
        if (p >= 0){
            if (ip < ipi && wc[p][1] > inner_cnt[ip]){
                goto inner_link;
            }
            inner_cnt[ipi] += wc[p][1];
            hbt[p][0] = ipi;
            if (t){
                hbt[p][1] = 1;
                ipi += 1;
            }
            p -= 1;
        }
        else{
inner_link:
            inner_cnt[ipi] += inner_cnt[ip];
            hbt[ip][2] = ipi;
            if (t){
                hbt[ip][3] = 1;
                ipi += 1;
            }
            ip += 1;
        }
        t ^= 1;
    }
    hbt[ipi - 1][2] = -1;
    free(inner_cnt); inner_cnt = NULL;
    for (t = 0; t < n; t++){
        hbt[wc[t][0]][4] = t;
    }
}

void hsoft_build(HSoft ** ahsf, int (*wc)[2], int v, int k){
    int i;
    HSoft * hsf = (HSoft*)calloc(1, sizeof(HSoft));
    *ahsf = hsf;

    hsf->v = v;
    hsf->k = k;
    hsf->hbt = (int (*)[5])calloc(v, sizeof(int[5]));
    hsf->tree = (double*)calloc(v * k, sizeof(double));

    // init v
    i = v * k - k;
    while (i-- > 0){
        hsf->tree[i] = ((rand() + 0.1) / (RAND_MAX + 0.1) - 0.5) / v;
    }

    // build huffmen tree for hierarchal softmax
    hsoft_build_tree(hsf->hbt, wc, v);
}

double hsoft_learn(HSoft * hsf, double *in, double *out, int tid, double alpha){
    int s = 0, t = 0, k = hsf->k, st[MXDEPT], sb[MXDEPT];
    double loss = 0.0, o = 0.0;

    memset(st, -1, sizeof(int) * MXDEPT);
    memset(sb, -1, sizeof(int) * MXDEPT);

    st[t] = hsf->hbt[hsf->hbt[tid][4]][0];
    sb[t] = hsf->hbt[hsf->hbt[tid][4]][1];

    while (-1 != st[t]){
        s = t;
        t = (t + 1) % MXDEPT;
        st[t] = hsf->hbt[st[s]][2];
        sb[t] = hsf->hbt[st[s]][3];
    }

    while (-1 != st[s]){
        o = 0.0;
        for (t = 0; t < k; t++){
            o += in[t] * hsf->tree[st[s] * k + t];
        }
        o = 1.0 / (1.0 + exp(-o));
        if (sb[s] == 1) loss -= log(o);
        else loss -= log(1.0 - o);
        o = sb[s] - o;
        if (fabs(o)  > 1e-9) for (t = 0; t < k; t++){
            out[t] += o * hsf->tree[st[s] * k + t];
            hsf->tree[st[s] * k + t] += alpha * o * in[t];
        }
        s -= 1;
        s = s < 0 ? (s + MXDEPT) : s;
    }
    return loss;
}

void hsoft_free(HSoft * hsf){
    if (hsf->hbt){
        free(hsf->hbt);
        hsf->hbt = NULL;
    }
    if (hsf->tree){
        free(hsf->tree);
        hsf->tree = NULL;
    }
    free(hsf);
}

void hsoft_save(HSoft * hsf, const char * outdir){
    char out[512] = {0};
    int i, j, off;
    FILE *fp = NULL;

    sprintf(out, "%s/inner_huff_treenode", outdir);
    if (NULL == (fp = fopen(out, "w"))){
        return;
    }

    for (i = 0; i < hsf->v; i++){
        off = i * hsf->k;
        fprintf(fp, "%.3f", hsf->tree[off]);
        for (j = 1; j < hsf->k; j++){
            fprintf(fp, "\t%.3f", hsf->tree[off + j]);
        }
        fprintf(fp, "\n");
    }
    fclose(fp);

    sprintf(out, "%s/huff_tree", outdir);
    if (NULL == (fp = fopen(out, "w"))) {
        return;
    }

    for (i = 0; i < hsf->v; i++){
        fprintf(fp, "%d\t%d\t%d\t%d\n"                \
                  , hsf->hbt[hsf->hbt[i][4]][0]       \
                  , hsf->hbt[hsf->hbt[i][4]][1]       \
                  , hsf->hbt[i][2], hsf->hbt[i][3]);
    }
    fclose(fp);
}

int hsoft_load(HSoft ** dhsf, const char * outdir, int k){
    HSoft * hsf = (HSoft*)calloc(1, sizeof(HSoft));
    *dhsf = hsf;
    hsf->k = k;

    int v, i;
    char *string, *token;
    char out[512] = {0};
    char buf[10000] = {0};
    FILE * fp = NULL;

    sprintf(out, "%s/huff_tree", outdir);
    if (NULL == (fp = fopen(out, "r"))){
        goto fail1;
    }
    v = 0;
    while (NULL != fgets(buf, 10000, fp)){
        v += 1;
    }
    rewind(fp);
    hsf->v = v;
    hsf->hbt = (int(*)[5])calloc(v, sizeof(int[5]));
    i = 0;
    while (NULL != fgets(buf, 10000, fp)){
        string = trim(buf, 3);
        hsf->hbt[i][0] = atoi(strsep(&string, "\t"));
        hsf->hbt[i][1] = atoi(strsep(&string, "\t"));
        hsf->hbt[i][2] = atoi(strsep(&string, "\t"));
        hsf->hbt[i][3] = atoi(strsep(&string, "\t"));
        hsf->hbt[i][4] = i;
        i += 1;
    }
    fclose(fp);

    sprintf(out, "%s/inner_huff_treenode", outdir);
    if (NULL == (fp = fopen(out, "r"))){
        goto fail2;
    }
    i = 0; 
    hsf->tree = (double *)calloc(v * k, sizeof(double));
    while (NULL != fgets(buf, 10000, fp)){
        string = trim(buf, 3);
        while (NULL != (token = strsep(&string, "\t"))){
            hsf->tree[i++] = atof(token);
        }
    }
    fclose(fp);

    return 0;

fail2:
    free(hsf->hbt);
    hsf->hbt = NULL;

fail1:
    free(hsf);
    *dhsf = hsf = NULL;
    return -1;
}


