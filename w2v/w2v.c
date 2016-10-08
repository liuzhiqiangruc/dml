/* ========================================================
 *   Copyright (C) 2016 All rights reserved.
 *   
 *   filename : w2v.c
 *   author   : liuzhiqiangruc@126.com
 *   date     : 2016-09-22
 *   info     : word2vec implementation of cbow using hsmax
 * ======================================================== */
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "w2v_config.h"
#include "hash.h"
#include "str.h"
#include "repo.h"
#include "w2v.h"

struct _w2v {
    int   voc_size;
    int   doc_size;
    int   tk_size;
    int   * roffs;
    int   * tokens;
    int   * b0;
    int   * b1;
    int   * p0;
    int   * p1;
    int   * indx;
    float * neu0;
    float * neu1;
    char (*idm)[KEY_SIZE];
    W2VConfig * wc;
};

static int wv_cmp(const void * a1, const void * a2){
    int * a = (int*)a1;
    int * b = (int*)a2;
    int c = a[1] - b[1];
    return c > 0 ? -1 : (c < 0 ? 1 : 0);
}

static void malloc_space(WV * wv){
    int i, v = wv->voc_size, r = wv->doc_size + 1, t = wv->tk_size;
    int k = wv->wc->get_k(wv->wc);
    wv->neu0   = (float*)calloc(v * k, sizeof(float));
    wv->neu1   = (float*)calloc(v * k, sizeof(float));
    wv->b0     = (int *) calloc(v, sizeof(int));
    wv->b1     = (int *) calloc(v, sizeof(int));
    wv->p0     = (int *) calloc(v, sizeof(int));
    wv->p1     = (int *) calloc(v, sizeof(int));
    wv->indx   = (int *) calloc(v, sizeof(int));
    wv->roffs  = (int *) calloc(r, sizeof(int));
    wv->tokens = (int *) calloc(t, sizeof(int));
    wv->idm    = (char(*)[KEY_SIZE])calloc(v, sizeof(char[KEY_SIZE]));
    i = v * k;
    while (i-- > 0){
        wv->neu0[i] = ((rand() + 0.1) / (RAND_MAX + 0.1) - 0.5) / k;
    }
}

static void build_tree(WV * wv, int (*wc)[2], int n){
    int p, ip, ipi, t = 0;
    int * inner_cnt = (int*)calloc(n, sizeof(int));
    qsort(wc, n, sizeof(int[2]), wv_cmp);
    p = n - 1;
    ip = ipi = 0;
    while (p >= 0 || ip < ipi){
        if (p >= 0){
            if (ip < ipi && wc[p][1] > inner_cnt[ip]){
                goto inner_link;
            }
            inner_cnt[ipi] += wc[p][1];
            wv->p0[p] = ipi;
            if (t){
                wv->b0[p] = 1;
                ipi += 1;
            }
            p -= 1;
        }
        else{
inner_link:
            inner_cnt[ipi] += inner_cnt[ip];
            wv->p1[ip] = ipi;
            if (t){
                wv->b1[ip] = 1;
                ipi += 1;
            }
            ip += 1;
        }
        t ^= 1;
    }
    wv->p1[ipi - 1] = -1; // root has no parent
    free(inner_cnt); inner_cnt = NULL;
    for (t = 0; t < n; t++){
        wv->indx[wc[t][0]] = t;
    }
}

WV * wv_create(int argc, char * argv[]){
    WV * wv = (WV*)calloc(1, sizeof(WV));
    wv->wc = init_config();
    if (0 != wv->wc->set(wv->wc, argc, argv)){
        wv->wc->help();
        return NULL;
    }
    return wv;
}

int wv_init(WV * wv) {
    FILE * fp = NULL;
    if (NULL == (fp = fopen(wv->wc->get_d(wv->wc), "r"))){
        fprintf(stderr, "can not open input file\n");
        return -1;
    }
    char buffer[LINE_LEN] = {'\0'};
    char *string = NULL, *token = NULL;
    Hash * whs = hash_create(1<<20, STRING);
    int id, tk = 0, dsize = 0, hsize = hash_size(whs);
    int (*tmp_cnt)[2] = NULL;
    int (*fea_cnt)[2] = (int(*)[2])calloc(hash_size(whs), sizeof(int[2]));
    while (NULL != fgets(buffer, LINE_LEN, fp)){
        string = trim(buffer, 3);
        while (NULL != (token = strsep(&string, "\t"))){
            id = hash_add(whs, token);
            if (id >= hsize){
                tmp_cnt = (int(*)[2])calloc(hash_size(whs), sizeof(int[2]));
                memmove(tmp_cnt, fea_cnt, sizeof(int[2]) * hsize);
                hsize = hash_size(whs);
                free(fea_cnt);
                fea_cnt = tmp_cnt;
            }
            fea_cnt[id][0]  = id;
            fea_cnt[id][1] += 1;
            tk += 1;
        }
        dsize += 1;
    }
    wv->voc_size = hash_cnt(whs);
    wv->doc_size = dsize;
    wv->tk_size  = tk;
    malloc_space(wv);
    build_tree(wv, fea_cnt, wv->voc_size);
    free(fea_cnt); fea_cnt = NULL;
    rewind(fp);
    dsize = tk = 0;
    while (NULL != fgets(buffer, LINE_LEN, fp)){
        string = trim(buffer, 3);
        while(NULL != (token = strsep(&string, "\t"))){
            id = hash_find(whs, token);
            wv->tokens[tk++] = id;
            if (wv->idm[id][0] == '\0'){
                strncpy(wv->idm[id], token, KEY_SIZE - 1);
            }
        }
        wv->roffs[++dsize] = tk;
    }
    fclose(fp);
    free(whs); whs = NULL;
    return 0;
}

static void learn_cw(WV * wv, float * cw, float * de, int tk){
    int k = wv->wc->get_k(wv->wc);
    int s = 0, t = 0, st[40] = {-1}, sb[40] = {-1};
    double loss = 0.0, learn_rate = wv->wc->get_alpha(wv->wc);
    sb[t] = wv->b0[wv->indx[tk]];
    st[t] = wv->p0[wv->indx[tk]];
    while (-1 != st[t]){
        s = t;
        t = (t + 1) % 40;
        st[t] = wv->p1[st[s]];
        sb[t] = wv->b1[st[s]];
    }
    while (-1 != st[s]){
        for (t = 0; t < k; t++){
            loss += cw[t] * wv->neu1[st[s] * k + t];
        }
        loss = 1 / (1 + exp(-loss));
        loss = learn_rate * (sb[s] - loss);
        for (t = 0; t < k; t++){
            de[t] += loss * wv->neu1[st[s] * k + t];
        }
        for (t = 0; t < k; t++){
            wv->neu1[st[s] * k + t] += loss * cw[t];
        }
        s -= 1;
        s = s < 0 ? (s + 40) : s;
    }
}

void wv_est(WV * wv){
    int w, d, id, ds, de, l, r, wi, m, tid, c, k, n = 7;
    w   = wv->wc->get_w(wv->wc);
    k = wv->wc->get_k(wv->wc);
    float * cw = (float *)malloc(k * sizeof(float));
    float * eu = (float *)malloc(k * sizeof(float));
    while (n-- > 0) for (d = 0; d < wv->doc_size; d++){
        memset(cw, 0, sizeof(float) * k);
        ds = wv->roffs[d];
        de = wv->roffs[d + 1];
        if (de - ds > 1) for (id = ds; id < de; id++){
            if (id - w / 2 < ds) l = ds;
            else l = id - w / 2;
            r = l + w;
            if (r > de) r = de;
            c = 0;
            for (wi = l; wi < r; wi++) if (wi != id) {
                tid = wv->tokens[wi];
                for (m = 0; m < k; m++){
                    cw[m] += wv->neu0[tid * k + m];
                }
                c += 1;
            }
            for (m = 0; m < k; m++){
                cw[m] /= c;
            }
            tid = wv->tokens[id];
            memset(eu, 0, sizeof(float) * k);
            learn_cw(wv, cw, eu, tid);
            for (wi = l; wi < r; wi++) if(wi != id){
                tid = wv->tokens[wi];
                for (m = 0; m < k; m++){
                    wv->neu0[tid * k + m] += eu[m];
                }
            }
        }
        progress(stderr, wv->doc_size, d + 1);
    }
}

void wv_save(WV * wv){
    char outf[512] = {'\0'};
    FILE * ofp = NULL;
    int i, t, v = wv->voc_size, k = wv->wc->get_k(wv->wc);
    sprintf(outf, "%s/vectors", wv->wc->get_o(wv->wc));
    if (NULL == (ofp = fopen(outf, "w"))){
        return;
    }
    for (i = 0; i < v; i++){
        fprintf(ofp, "%s", wv->idm[i]);
        for (t = 0; t < k; t++){
            fprintf(ofp, "\t%.3f", wv->neu0[i * k + t]);
        }
        fprintf(ofp, "\n");
    }
    fclose(ofp);
}

void wv_free(WV * wv){
    if (wv){
        if (wv->roffs){
            free(wv->roffs);
            wv->roffs = NULL;
        }
        if (wv->tokens){
            free(wv->tokens);
            wv->tokens = NULL;
        }
        if (wv->b0){
            free(wv->b0);
            wv->b0 = NULL;
        }
        if (wv->b1){
            free(wv->b1);
            wv->b1 = NULL;
        }
        if (wv->p0){
            free(wv->p0);
            wv->p1 = NULL;
        }
        if (wv->p1){
            free(wv->p1);
            wv->p1 = NULL;
        }
        if (wv->indx){
            free(wv->indx);
            wv->indx = NULL;
        }
        if (wv->neu0){
            free(wv->neu0);
            wv->neu0 = NULL;
        }
        if (wv->neu1){
            free(wv->neu1);
            wv->neu1 = NULL;
        }
        if (wv->idm){
            free(wv->idm);
            wv->idm = NULL;
        }
        if (wv->wc){
            wv->wc->free(wv->wc);
            wv->wc = NULL;
        }
        free(wv);
    }
}

int wv_dsize(WV * wv){
        return wv->doc_size;
}

int wv_vsize(WV * wv){
        return wv->voc_size;
}

int wv_tsize(WV * wv){
        return wv->tk_size;
}
