/* ========================================================
 *   Copyright (C) 2016 All rights reserved.
 *   
 *   filename : d2v.c
 *   author   : liuzhiqiangruc@126.com
 *   date     : 2016-10-21
 *   info     : 
 * ======================================================== */
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "d2v_config.h"
#include "hash.h"
#include "str.h"
#include "repo.h"
#include "d2v.h"

#define KEY_SIZE 64
#define LINE_LEN 1024
#define MLEN     8192
#define MTDEPT   40

struct  _d2v {
    int   v_size;
    int   d_size;
    int   t_size;
    int   * roffs;
    int   * tokens;
    int  (*hbt)[5];
    float * neu0;
    float * neu1;
    char (*idm)[KEY_SIZE];
    D2VConfig * dc;
};

static int dv_cmp(const void * a1, const void * a2){
    int * a = (int*)a1;
    int * b = (int*)a2;
    int c = a[1] - b[1];
    return c > 0 ? -1 : (c < 0 ? 1 : 0);
}

static void malloc_space(DV * dv){
    int i, v = dv->v_size, r = dv->d_size, t = dv->t_size;
    int k = dv->dc->get_k(dv->dc);
    dv->neu0   = (float*)    calloc(r * k, sizeof(float));
    dv->neu1   = (float*)    calloc(v * k, sizeof(float));
    dv->hbt    = (int(*)[5]) calloc(v,     sizeof(int[5]));
    dv->roffs  = (int *)     calloc(r + 1, sizeof(int));
    dv->tokens = (int *)     calloc(t,     sizeof(int));
    dv->idm    = (char(*)[KEY_SIZE])calloc(v, sizeof(char[KEY_SIZE]));
    i = r * k;
    while (i-- > 0){
        dv->neu0[i] = ((rand() + 0.1) / (RAND_MAX + 0.1) - 0.5) / k;
    }
}

static void build_tree(DV * dv, int (*dc)[2], int n){
    int p, ip, ipi, t = 0;
    int * inner_cnt = (int*)calloc(n, sizeof(int));
    qsort(dc, n, sizeof(int[2]), dv_cmp);
    p = n - 1;
    ip = ipi = 0;
    while (p >= 0 || ip < ipi){
        if (p >= 0){
            if (ip < ipi && dc[p][1] > inner_cnt[ip]){
                goto inner_link;
            }
            inner_cnt[ipi] += dc[p][1];
            dv->hbt[p][0] = ipi;
            if (t){
                dv->hbt[p][1] = 1;
                ipi += 1;
            }
            p -= 1;
        }
        else{
inner_link:
            inner_cnt[ipi] += inner_cnt[ip];
            dv->hbt[ip][2] = ipi;
            if (t){
                dv->hbt[ip][3] = 1;
                ipi += 1;
            }
            ip += 1;
        }
        t ^= 1;
    }
    dv->hbt[ipi - 1][2] = -1; // root has no parent
    free(inner_cnt); inner_cnt = NULL;
    for (t = 0; t < n; t++){
        dv->hbt[dc[t][0]][4] = t;
    }
}

static int dv_load_data(DV * dv){
    FILE * fp = NULL;
    if (NULL == (fp = fopen(dv->dc->get_d(dv->dc), "r"))){
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
    dv->v_size = hash_cnt(whs);
    dv->d_size = dsize;
    dv->t_size = tk;
    malloc_space(dv);
    build_tree(dv, fea_cnt, dv->v_size);
    free(fea_cnt); fea_cnt = NULL;
    rewind(fp);
    dsize = tk = 0;
    while (NULL != fgets(buffer, LINE_LEN, fp)){
        string = trim(buffer, 3);
        while(NULL != (token = strsep(&string, "\t"))){
            id = hash_find(whs, token);
            dv->tokens[tk++] = id;
            if (dv->idm[id][0] == '\0'){
                strncpy(dv->idm[id], token, KEY_SIZE - 1);
            }
        }
        dv->roffs[++dsize] = tk;
    }
    fclose(fp);
    free(whs); whs = NULL;
    return 0;
}

static int dv_load_model(DV * dv) {
    int i, r = dv->d_size, v = dv->v_size, k = dv->dc->get_k(dv->dc);
    char *outdir = dv->dc->get_o(dv->dc);
    char *string = NULL, *token = NULL;
    char outs[512] = {'\0'}, buf[MLEN] = {'\0'};
    FILE * fp = NULL;
    sprintf(outs, "%s/dvectors", outdir);
    if (NULL == (fp = fopen(outs, "r"))){
        return -1;
    }
    if (r == 0){
        while(NULL != fgets(buf, MLEN, fp)){
            r += 1;
        }
        dv->d_size = r;
        rewind(fp);
    }
    if (!dv->neu0){
        dv->neu0 = (float*)calloc(r * k, sizeof(float));
    }
    i = 0;
    while (NULL != fgets(buf, MLEN, fp)){
        string = trim(buf, 3);
        while(NULL != (token = strsep(&string, "\t"))){
            dv->neu0[i++] = atof(token);
        }
    }
    fclose(fp);
    sprintf(outs, "%s/noleaf", outdir);
    if (NULL == (fp = fopen(outs, "r"))){
        return -1;
    }
    if (v == 0){
        while(NULL != fgets(buf, MLEN, fp)){
            v += 1;
        }
        dv->v_size = v;
        rewind(fp);
    }
    if (!dv->neu1){
        dv->neu1 = (float*)calloc(v * k, sizeof(float));
    }
    i = 0;
    while (NULL != fgets(buf, MLEN, fp)){
        string = trim(buf, 3);
        while(NULL != (token = strsep(&string, "\t"))){
            dv->neu1[i++] = atof(token);
        }
    }
    fclose(fp);
    sprintf(outs, "%s/index", outdir);
    if (NULL == (fp = fopen(outs, "r"))){
        return -1;
    }
    if (!dv->hbt){
        dv->hbt = (int(*)[5]) calloc(v, sizeof(int[5]));
    }
    if (!dv->idm){
        dv->idm = (char(*)[KEY_SIZE])calloc(v, sizeof(char[KEY_SIZE]));
    }
    i = 0;
    while(NULL != fgets(buf, MLEN, fp)){
        string = trim(buf, 3);
        token = strsep(&string, "\t");
        strncpy(dv->idm[i], token, KEY_SIZE - 1);
        dv->hbt[i][0] = atoi(strsep(&string, "\t"));
        dv->hbt[i][1] = atoi(strsep(&string, "\t"));
        dv->hbt[i][2] = atoi(strsep(&string, "\t"));
        dv->hbt[i][3] = atoi(strsep(&string, "\t"));
        dv->hbt[i][4] = i;
        i += 1;
    }
    fclose(fp);
    return 0;
}

static void learn_cw(DV * dv, float * cw, float * de, int tk){
    int k = dv->dc->get_k(dv->dc);
    int s = 0, t = 0, st[MTDEPT], sb[MTDEPT];
    double loss = 0.0, learn_rate = dv->dc->get_alpha(dv->dc);
    memset(st, -1, sizeof(int) * MTDEPT);
    memset(sb, -1, sizeof(int) * MTDEPT);
    st[t] = dv->hbt[dv->hbt[tk][4]][0];
    sb[t] = dv->hbt[dv->hbt[tk][4]][1];
    while (-1 != st[t]){
        s = t;
        t = (t + 1) % MTDEPT;
        st[t] = dv->hbt[st[s]][2];
        sb[t] = dv->hbt[st[s]][3];
    }
    while (-1 != st[s]){
        loss = 0.0;
        for (t = 0; t < k; t++){
            loss += cw[t] * dv->neu1[st[s] * k + t];
        }
        loss = learn_rate * (sb[s] - 1.0 / (1.0 + exp(-loss)));
        if (fabs(loss) > 1e-9) for (t = 0; t < k; t++){
            de[t] += loss * dv->neu1[st[s] * k + t];
            if (dv->dc->get_t(dv->dc) != 2)
                dv->neu1[st[s] * k + t] += loss * cw[t];
        }
        s -= 1;
        s = s < 0 ? (s + MTDEPT) : s;
    }
}

DV * dv_create(int argc, char * argv[]){
    DV * dv = (DV*)calloc(1, sizeof(DV));
    dv->dc = init_d_config();
    if (0 != dv->dc->set(dv->dc, argc, argv)){
        dv->dc->help();
        return NULL;
    }
    return dv;
}

int dv_init(DV * dv){
    int c = dv_load_data(dv);
    int t = dv->dc->get_t(dv->dc);
    if (c == 0 && t == 1){
        dv_load_model(dv);
    }
    return 0;
}

void dv_est(DV * dv){
    int d, id, ds, de, m, tid, k, n;
    float *cw = NULL, *eu = NULL;
    k = dv->dc->get_k(dv->dc);
    n = dv->dc->get_n(dv->dc);
    eu = (float *)malloc(k * sizeof(float));
    while (n-- > 0) for (d = 0; d < dv->d_size; d++){
        cw = dv->neu0 + d * k;
        ds = dv->roffs[d];
        de = dv->roffs[d + 1];
        for (id = ds; id < de; id++){
            tid = dv->tokens[id];
            memset(eu, 0, sizeof(float) * k);
            learn_cw(dv, cw, eu, tid);
            for (m = 0; m < k; m++){
                cw[m] += eu[m];
            }
        }
        progress(stderr, dv->d_size, d + 1);
    }
    free(eu); eu = NULL;
}

void dv_save(DV * dv){
    char outf[512] = {'\0'};
    FILE * ofp = NULL;
    int i, t, d = dv->d_size, v = dv->v_size, k = dv->dc->get_k(dv->dc);
    sprintf(outf, "%s/dvectors", dv->dc->get_o(dv->dc));
    if (NULL == (ofp = fopen(outf, "w"))){
        return;
    }
    for (i = 0; i < d; i++){
        fprintf(ofp, "%.3f", dv->neu0[i * k]);
        for (t = 1; t < k; t++){
            fprintf(ofp, "\t%.3f", dv->neu0[i * k + t]);
        }
        fprintf(ofp, "\n");
    }
    fclose(ofp);
    sprintf(outf, "%s/noleaf", dv->dc->get_o(dv->dc));
    if (NULL == (ofp = fopen(outf, "w"))){
        return;
    }
    for (i = 0; i < v; i++){
        fprintf(ofp, "%.3f", dv->neu1[i * k]);
        for (t = 1; t < k; t++){
            fprintf(ofp, "\t%.3f", dv->neu1[i * k + t]);
        }
        fprintf(ofp, "\n");
    }
    fclose(ofp);
    sprintf(outf, "%s/index", dv->dc->get_o(dv->dc));
    if (NULL == (ofp = fopen(outf, "w"))){
        return;
    }
    for (i = 0; i < v; i++){
        fprintf(ofp, "%s\t%d\t%d\t%d\t%d\n"       \
                   , dv->idm[i]                   \
                   , dv->hbt[dv->hbt[i][4]][0]    \
                   , dv->hbt[dv->hbt[i][4]][1]    \
                   , dv->hbt[i][2], dv->hbt[i][3]);
    }
    fclose(ofp);
}

void dv_free(DV * dv){
    if (dv){
        if (dv->roffs){
            free(dv->roffs);
            dv->roffs = NULL;
        }
        if (dv->tokens){
            free(dv->tokens);
            dv->tokens = NULL;
        }
        if (dv->hbt){
            free(dv->hbt);
            dv->hbt = NULL;
        }
        if (dv->neu0){
            free(dv->neu0);
            dv->neu0 = NULL;
        }
        if (dv->neu1){
            free(dv->neu1);
            dv->neu1 = NULL;
        }
        if (dv->idm){
            free(dv->idm);
            dv->idm = NULL;
        }
        if (dv->dc){
            dv->dc->free(dv->dc);
            dv->dc = NULL;
        }
        free(dv);
    }
}

int dv_dsize(DV * dv){
    return dv->d_size;
}

int dv_vsize(DV * dv){
    return dv->v_size;
}

int dv_tsize(DV * dv){
    return dv->t_size;
}
