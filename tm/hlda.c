/* ========================================================
 *   Copyright (C) 2016 All rights reserved.
 *   
 *   filename : hlda.c
 *   author   : liuzhiqiangruc@126.com
 *   date     : 2016-07-11
 *   info     : 
 * ======================================================== */
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "hash.h"
#include "str.h"
#include "lda.h"

#define LDA_LINE_LEN 1024

typedef struct {
    unsigned short prev;
    unsigned short next;
    unsigned int   count;
} ModelEle;

struct _lda{
    unsigned int d;                /* doc_size  */
    unsigned int l;                /* loc_size  */
    unsigned int v;                /* word_size */
    unsigned int t;                /* tkn_size  */
    char (*id_d_map)[KEY_SIZE];    /* doc_map   */
    char (*id_v_map)[KEY_SIZE];    /* word_map  */
    char (*id_l_map)[KEY_SIZE];    /* loc_map   */
    int (*tokens)[6];              /* tokens  : {uid, lid, vid, xid, tid, link */
    int (*doc_cnt)[2];             /* doc_cnt : {tpcnt, locnt}  */
    int  *doc_ent;                 /* doc ent   */
    ModelEle * nd;                 /* doc_topic */
    ModelEle * nw;                 /* word_topic*/
    int * wl;                      /* word_loc  */
    int * nkw;                     /* topic_cnt */
    int * ln;                      /* loc_cnt   */
    ParamLda p;                    /* param     */
};

static void malloc_space(Lda * lda){
    lda->id_d_map = (char(*)[KEY_SIZE])calloc(lda->d,       sizeof(char[KEY_SIZE]));
    lda->id_l_map = (char(*)[KEY_SIZE])calloc(lda->l,       sizeof(char[KEY_SIZE]));
    lda->id_v_map = (char(*)[KEY_SIZE])calloc(lda->v,       sizeof(char[KEY_SIZE]));
    lda->tokens   = (int(*)[6])        calloc(lda->t,       sizeof(int[6]));
    lda->doc_cnt  = (int(*)[2])        calloc(lda->d,       sizeof(int[2]));
    lda->doc_ent  = (int *)            malloc(lda->d   *    sizeof(int));
    lda->ln       = (int *)            calloc(lda->l,       sizeof(int));
    lda->nkw      = (int *)            calloc(lda->p.k + 1, sizeof(int));
    lda->wl       = (int *)            calloc(lda->v * lda->l,         sizeof(int));
    lda->nd       = (ModelEle*)        calloc(lda->d * (lda->p.k + 1), sizeof(ModelEle));
    lda->nw       = (ModelEle*)        calloc(lda->v * (lda->p.k + 1), sizeof(ModelEle));
    memset(lda->doc_ent, -1, lda->d * sizeof(int));
}

static void fullfill_param(Lda * lda){
    int i, uid, lid, vid, xid, tid, d, v, offs, p, k;
    for (i = 0; i < lda->t; i++){
        uid = lda->tokens[i][0];
        lid = lda->tokens[i][1];
        vid = lda->tokens[i][2];
        xid = lda->tokens[i][3];
        tid = lda->tokens[i][4];
        if (xid == 0) {
            lda->nd[uid * (lda->p.k + 1) + tid].count += 1;
            lda->nw[vid * (lda->p.k + 1) + tid].count += 1;
            lda->nkw[tid] += 1;
            lda->doc_cnt[uid][0] += 1;
        }
        if (xid == 1){
            lda->wl[vid * lda->l + lid] += 1;
            lda->ln[lid] += 1;
            lda->doc_cnt[uid][1] += 1;
        }
    }
    for (d = 0; d < lda->d; d++){
        offs = d * (lda->p.k + 1);
        p = 0;
        for (k = 1; k <= lda->p.k; k++){
            if (lda->nd[offs + k].count > 0){
                lda->nd[offs + p].next = k;
                lda->nd[offs + k].prev = p;
                p = k;
            }
        }
        lda->nd[offs + p].next = 0;
        lda->nd[offs].prev = p;
    }
    for (v = 0; v < lda->v; v++){
        offs = v * (lda->p.k + 1);
        p = 0;
        for (k = 1; k <= lda->p.k; k++){
            if (lda->nw[offs + k].count > 0){
                lda->nw[offs + p].next = k;
                lda->nw[offs + k].prev = p;
                p = k;
            }
        }
        lda->nw[offs + p].next = 0;
        lda->nw[offs].prev = p;
    }
    save_lda(lda, 0);
}

static double init_e(Lda * lda, double * pse){
    int k;
    double e = 0.0;
    double ab = lda->p.a * lda->p.b;
    double vb = lda->p.b * lda->v;
    for (k = 1; k <= lda->p.k; k++){
        pse[k] = ab / (vb + lda->nkw[k]);
        e += pse[k];
    }
    return e;
}

static double init_f(Lda * lda, double * psd, int d){
    double f = 0.0, vb = lda->p.b * lda->v;
    int offs = d * (lda->p.k + 1), k;
    k = lda->nd[offs].next;
    while (k != 0){
        psd[k] = lda->p.b * (lda->nd[offs + k].count) / (vb + lda->nkw[k]);
        f += psd[k];
        k = lda->nd[offs + k].next;
    }
    return f;
}

static double init_g(Lda * lda, double * psv, int d, int v){
    double g = 0.0, vb = lda->p.b * lda->v;
    int doffs = d * (lda->p.k + 1), voffs = v * (lda->p.k + 1);
    int k = lda->nw[voffs].next;
    while (k != 0){
        psv[k] = 1.0 * lda->nw[voffs + k].count \
                     * (lda->p.a + lda->nd[doffs + k].count) \
                     / (vb + lda->nkw[k]);
        g += psv[k];
        k = lda->nw[voffs + k].next;
    }
    return g;
}

static double update_e(Lda * lda, double * pse, int t, int delta){
    double ab = lda->p.a * lda->p.b;
    double vb = lda->p.b * lda->v;
    double tmp = 0.0;
    tmp += ab / (vb + lda->nkw[t]);
    tmp -= ab / (vb + lda->nkw[t] + delta);
    pse[t] += tmp;
    return tmp;
}

static double update_f(Lda * lda, double * psd, int d, int t, int delta){
    double vb = lda->p.b * lda->v;
    double tmp = 0.0;
    int doffs = d * (lda->p.k + 1);
    tmp += lda->p.b * lda->nd[doffs + t].count / (vb + lda->nkw[t]);
    tmp -= lda->p.b * (lda->nd[doffs + t].count + delta) / (vb + lda->nkw[t] + delta);
    psd[t] += tmp;
    return tmp;
}

static void off_tp_k(Lda * lda, int d, int v, int t){
    int voffs = v * (lda->p.k + 1);
    int doffs = d * (lda->p.k + 1);
    int p, n;
    lda->nd[doffs + t].count -= 1;
    lda->nw[voffs + t].count -= 1;
    lda->nkw[t]              -= 1;
    lda->doc_cnt[d][0]       -= 1;
    if (lda->nd[doffs + t].count == 0){
        p = lda->nd[doffs + t].prev;
        n = lda->nd[doffs + t].next;
        lda->nd[doffs + p].next = n;
        lda->nd[doffs + n].prev = p;
        lda->nd[doffs + t].next = 0;
        lda->nd[doffs + t].prev = 0;
    }
    if (lda->nw[voffs + t].count == 0){
        p = lda->nw[voffs + t].prev;
        n = lda->nw[voffs + t].next;
        lda->nw[voffs + p].next = n;
        lda->nw[voffs + n].prev = p;
        lda->nw[voffs + t].next = 0;
        lda->nw[voffs + t].prev = 0;
    }
}

static void addon_tp_k(Lda * lda, int d, int v, int k, int t){
    int doffs = d * (lda->p.k + 1);
    int voffs = v * (lda->p.k + 1);
    if (lda->nd[doffs + k].count == 0){
        lda->nd[doffs + lda->nd[doffs].prev].next = k;
        lda->nd[doffs + k].prev = lda->nd[doffs].prev;
        lda->nd[doffs + k].next = 0;
        lda->nd[doffs].prev = k;
    }
    if (lda->nw[voffs + k].count == 0){
        lda->nw[voffs + lda->nw[voffs].prev].next = k;
        lda->nw[voffs + k].prev = lda->nw[voffs].prev;
        lda->nw[voffs + k].next = 0;
        lda->nw[voffs].prev = k;
    }
    lda->nd[doffs + k].count += 1;
    lda->nw[voffs + k].count += 1;
    lda->nkw[k]              += 1;
    lda->doc_cnt[d][0]       += 1;
    lda->tokens[t][3]         = 0;
    lda->tokens[t][4]         = k;
}

static void off_lc_l(Lda * lda, int d, int v, int l){
    lda->wl[v * lda->l + l] -= 1;
    lda->ln[l]              -= 1;
    lda->doc_cnt[d][1]      -= 1;
}

static void addon_lc_l(Lda * lda, int d, int v, int l, int t){
    lda->wl[v * lda->l + l] += 1;
    lda->ln[l]              += 1;
    lda->doc_cnt[d][1]      += 1;
    lda->tokens[t][3]        = 1;
    lda->tokens[t][4]        =-1;
}

static int sample_k(Lda * lda, double * pse
                             , double * psd
                             , double * psv
                             , double e
                             , double f
                             , double g
                             , double r
                             , int d
                             , int v){
    double tmp = 0.0;
    int k, doffs = d * (lda->p.k + 1), voffs = v * (lda->p.k + 1);
    if (r < e){
        for (k = 1; k <= lda->p.k; k++){
            tmp += pse[k];
            if (tmp > r){
                break;
            }
        }
    }
    else if (r < e + f){
        r -= e;
        k = lda->nd[doffs].next;
        while (k != 0){
            tmp += psd[k];
            if (tmp > r ){
                break;
            }
            k = lda->nd[doffs + k].next;
        }
    }
    else{
        r -= (e + f);
        k = lda->nw[voffs].next;
        while (k != 0){
            tmp += psv[k];
            if (tmp > r){
                break;
            }
            k = lda->nw[voffs + k].next;
        }
    }
    return k;
}

static int gibbs_sample(Lda * lda){
    double ta = lda->p.a * lda->p.k;
    double e, f, g, s, s1, s2, r, tpr;
    double *pse = (double*)malloc(sizeof(double) * (lda->p.k + 1));
    double *psd = (double*)malloc(sizeof(double) * (lda->p.k + 1));
    double *psv = (double*)malloc(sizeof(double) * (lda->p.k + 1));
    int i, k, d, l, v, x, t;
    e = init_e(lda, pse);
    for (d = 0; d < lda->d; d++){
        f = init_f(lda, psd, d);
        i = lda->doc_ent[d];
        while (i != -1){
            l = lda->tokens[i][1];
            v = lda->tokens[i][2];
            x = lda->tokens[i][3];
            t = lda->tokens[i][4];
            if (x == 0){
                off_tp_k(lda, d, v, t);
                e += update_e(lda, pse, t, 1);
                f += update_f(lda, psd, d, t, 1);
            }
            else {
                off_lc_l(lda, d, v, l);
            }
            g = init_g(lda, psv, d, v);
            tpr = (lda->p.g0 + lda->doc_cnt[d][0]) \
                / (lda->p.g0 + lda->doc_cnt[d][0]+ \
                   lda->p.g1 + lda->doc_cnt[d][1]);
            s1 = (e + f + g) * tpr / (ta + lda->doc_cnt[d][0]);
            s2 = (1.0 - tpr) * (lda->p.b + lda->wl[v * lda->l + l]) \
                             / (lda->p.b * lda->v + lda->ln[l]);
            s = s1 + s2;
            r = s * (0.1 + rand()) / (0.1 + RAND_MAX);
            if (s1 <= r){ 
                addon_lc_l(lda, d, v, l, i);
            }
            else{
                r = r / tpr * (ta + lda->doc_cnt[d][0]);
                k = sample_k(lda, pse, psd, psv, e, f, g, r, d, v);
                if (k == 0){
                    fprintf(stderr, "\nsample failed\n");
                    return -1;
                }
                addon_tp_k(lda, d, v, k, i);
                e += update_e(lda, pse, k, -1);
                f += update_f(lda, psd, d, k, -1);
            }
            i = lda->tokens[i][5];
        }
    }
    free(pse); pse = NULL;
    free(psd); psd = NULL;
    free(psv); psv = NULL;
    return 0;
}

Lda * create_lda(ParamLda p){
    Lda * lda = (Lda*)calloc(1, sizeof(Lda));
    lda->p = p;
    return lda;
}

int init_lda(Lda * lda){
    FILE * fp = NULL;
    if (NULL == (fp = fopen(lda->p.in_file, "r"))){
        fprintf(stderr, "can not open input file\n");
        return -1;
    }
    char buffer[LDA_LINE_LEN] = {'\0'};
    char *string, *token;
    int tk, uid, vid, lid, xid, tid;
    Hash * uhs = hash_create(1<<20, STRING);
    Hash * vhs = hash_create(1<<20, STRING);
    Hash * lhs = hash_create(1<<20, STRING);
    tk = 0;
    while(NULL != fgets(buffer, LDA_LINE_LEN, fp)){
        string = trim(buffer, 3);
        token = strsep(&string, "\t");
        hash_add(uhs, token);
        token = strsep(&string, "\t");
        hash_add(lhs, token);
        token = strsep(&string, "\t");
        hash_add(vhs, token);
        tk += 1;
    }
    lda->d = hash_cnt(uhs);
    lda->l = hash_cnt(lhs);
    lda->v = hash_cnt(vhs);
    lda->t = tk;
    tk = 0;
    malloc_space(lda);
    rewind(fp);
    while(NULL != fgets(buffer, LDA_LINE_LEN, fp)){
        string = trim(buffer, 3);
        token = strsep(&string, "\t");
        uid   = hash_find(uhs, token);
        if (lda->id_d_map[uid][0] == '\0'){
            strncpy(lda->id_d_map[uid], token, KEY_SIZE - 1);
        }
        lda->tokens[tk][0] = uid;
        token = strsep(&string, "\t");
        lid   = hash_find(lhs, token);
        if (lda->id_l_map[lid][0] == '\0'){
            strncpy(lda->id_l_map[lid], token, KEY_SIZE - 1);
        }
        lda->tokens[tk][1] = lid;
        token = strsep(&string, "\t");
        vid   = hash_find(vhs, token);
        if (lda->id_v_map[vid][0] == '\0'){
            strncpy(lda->id_v_map[vid], token, KEY_SIZE - 1);
        }
        lda->tokens[tk][2] = vid;
        xid = 0;
        token = strsep(&string, "\t");
        if (token){
            xid = atoi(token);
        }
        lda->tokens[tk][3] = xid;
        tid = (int) ((0.1 + rand()) / (0.1 + RAND_MAX) * lda->p.k);
        tid += 1;
        if (xid == 1){
            tid = -1;     // no use
        }
        else if (token){
            token = strsep(&string, "\t");
            tid = atoi(token);
        }
        lda->tokens[tk][4] = tid;
        lda->tokens[tk][5] = lda->doc_ent[uid];
        lda->doc_ent[uid] = tk;
        tk += 1;
    }
    fclose(fp);
    hash_free(uhs); uhs = NULL;
    hash_free(vhs); vhs = NULL;
    hash_free(lhs); lhs = NULL;
    return 0;
}

void est_lda(Lda *lda) {
    fullfill_param(lda);
    for (int n = 1; n <= lda->p.niters; n++){
        long sec1 = time(NULL);
        gibbs_sample(lda);
        long sec2 = time(NULL);
        fprintf(stderr, "iter %d done, using %ld seconds\n", n, sec2 - sec1);
        if (n % lda->p.savestep == 0){
            save_lda(lda, n);
        }
    }
}

void save_lda(Lda *lda, int n) {
    FILE *fp = NULL;
    char nd_file[512];
    char nw_file[512];
    char tk_file[512];
    char wl_file[512];
    int d, v, t, offs, k, l;
    if (n < lda->p.niters){
        sprintf(nd_file,  "%s/%d_doc_topic",  lda->p.out_dir, n);
        sprintf(nw_file,  "%s/%d_word_topic", lda->p.out_dir, n);
        sprintf(tk_file,  "%s/%d_token_topic",lda->p.out_dir, n);
        sprintf(wl_file,  "%s/%d_word_local", lda->p.out_dir, n);
    }
    else{
        sprintf(nd_file,  "%s/%s_doc_topic",  lda->p.out_dir, "f");
        sprintf(nw_file,  "%s/%s_word_topic", lda->p.out_dir, "f");
        sprintf(tk_file,  "%s/%s_token_topic",lda->p.out_dir, "f");
        sprintf(wl_file,  "%s/%s_word_local", lda->p.out_dir, "f");
    }
    //output for nd
    if (NULL == (fp = fopen(nd_file, "w"))) {
        fprintf(stderr, "can not open file \"%s\"", nd_file);
        return;
    }
    for (d = 0; d < lda->d; d++) {
        fprintf(fp, "%s", lda->id_d_map[d]);
        offs = d * (lda->p.k + 1);
        for (k = 1; k <= lda->p.k; k++) {
            fprintf(fp, "\t%d", lda->nd[offs + k].count);
        }
        fprintf(fp, "\n");
    }
    fclose(fp);
    //output for nw
    if (NULL == (fp = fopen(nw_file, "w"))) {
        fprintf(stderr, "can not open file \"%s\"", nw_file);
        return;
    }
    for (v = 0; v < lda->v; v++) {
        fprintf(fp, "%s", lda->id_v_map[v]);
        offs = v * (lda->p.k + 1);
        for (k = 1; k <= lda->p.k; k++) {
            fprintf(fp, "\t%d", lda->nw[offs + k].count);
        }
        fprintf(fp, "\n");
    }
    fclose(fp);
    //output for tk
    if (NULL == (fp = fopen(tk_file, "w"))) {
        fprintf(stderr, "can not open file \"%s\"", tk_file);
        return;
    }
    for (t = 0; t < lda->t; t++) {
        fprintf(fp, "%s\t%s\t%s\t%d\t%d\n",   lda->id_d_map[lda->tokens[t][0]],  \
                                              lda->id_l_map[lda->tokens[t][1]],  \
                                              lda->id_v_map[lda->tokens[t][2]],  \
                                              lda->tokens[t][3],                 \
                                              lda->tokens[t][4]);
    }
    fclose(fp);
    //output for wl
    if (NULL == (fp = fopen(wl_file, "w"))){
        fprintf(stderr, "can not open file \"%s\"", tk_file);
        return;
    }
    fprintf(fp, "word:local");
    for (l = 0; l < lda->l; l++){
        fprintf(fp, "\t%s", lda->id_l_map[l]);
    }
    fprintf(fp, "\n");
    for (v = 0; v < lda->v; v++){
        fprintf(fp, "%s", lda->id_v_map[v]);
        for (l = 0; l < lda->l; l++){
            fprintf(fp, "\t%d", lda->wl[v * lda->l + l]);
        }
        fprintf(fp, "\n");
    }
    fclose(fp);
}

void free_lda(Lda *lda) {
    if (lda) {
        if (lda->id_d_map) {
            free(lda->id_d_map);
            lda->id_d_map = NULL;
        }
        if (lda->id_v_map) {
            free(lda->id_v_map);
            lda->id_v_map = NULL;
        }
        if (lda->id_l_map){
            free(lda->id_l_map);
            lda->id_l_map = NULL;
        }
        if (lda->tokens){
            free(lda->tokens);
            lda->tokens = NULL;
        }
        if (lda->doc_cnt){
            free(lda->doc_cnt);
            lda->doc_cnt = NULL;
        }
        if (lda->doc_ent){
            free(lda->doc_ent);
            lda->doc_ent = NULL;
        }
        if (lda->nd) {
            free(lda->nd);
            lda->nd = NULL;
        }
        if (lda->nw) {
            free(lda->nw);
            lda->nw = NULL;
        }
        if (lda->nkw) {
            free(lda->nkw);
            lda->nkw = NULL;
        }
        if (lda->wl){
            free(lda->wl);
            lda->wl = NULL;
        }
        if (lda->ln){
            free(lda->ln);
            lda->ln = NULL;
        }
        free(lda);
    }
}

int lda_d(Lda * lda){
    return lda->d;
}

int lda_v(Lda * lda){
    return lda->v;
}

int lda_t(Lda * lda){
    return lda->t;
}
