/* ========================================================
 *   Copyright (C) 2016 All rights reserved.
 *   
 *   filename : hsoft.c
 *   author   : ***
 *   date     : 2016-12-06
 *   info     : 
 * ======================================================== */

#include <math.h>
#include <stdlib.h>
#include <string.h>
#include "hsoft.h"

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

void hsoft_learn(HSoft * hsf, double *in, double *out, int tid, double alpha){
    int s = 0, t = 0, k = hsf->k, st[MXDEPT], sb[MXDEPT];
    double loss = 0.0;

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
        loss = 0.0;
        for (t = 0; t < k; t++){
            loss += in[t] * hsf->tree[st[s] * k + t];
        }
        loss = (sb[s] - 1.0 / (1.0 + exp(-loss)));
        if (fabs(loss)  > 1e-9) for (t = 0; t < k; t++){
            out[t] += loss * hsf->tree[st[s] * k + t];
            hsf->tree[st[s] * k + t] += alpha * loss * in[t];
        }
        s -= 1;
        s = s < 0 ? (s + MXDEPT) : s;
    }
}
