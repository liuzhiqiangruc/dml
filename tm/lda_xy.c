/* ========================================================
 *   Copyright (C) 2016 All rights reserved.
 *   
 *   filename : lda_xy.c
 *   author   : liuzhiqiangruc@126.com
 *   date     : 2016-06-22
 *   info     : 
 * ======================================================== */
#include <time.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "hash.h"
#include "str.h"
#include "lda.h"

#define LDA_LINE_LEN 1024
/* ----------------------------------------------------
 * model struct for Lda
 * ---------------------------------------------------- */
struct _lda {
    int d;                         /* doc size          */
    int t;                         /* token size        */
    int v;                         /* dict size         */
    char (*id_doc_map)[KEY_SIZE];  /* doc id map        */
    char (*id_v_map)[KEY_SIZE];    /* word id map       */
    int  (*tokens)[3];             /* tokens            */
    double (*xy)[2];               /* pos of token      */
    int * nd;                      /* theta             */
    int * nw;                      /* phi               */
    int * nkw;                     /* token n of topic  */
    double (*disp)[11];            /* for pos info      */
    ParamLda p;                    /* lda parameter     */
};

static void malloc_space(Lda * lda){
    lda->id_doc_map = (char (*)[KEY_SIZE]) malloc(sizeof(char[KEY_SIZE]) * lda->d);
    memset(lda->id_doc_map, 0, sizeof(char[KEY_SIZE]) * lda->d);
    lda->id_v_map = (char (*)[KEY_SIZE]) malloc(sizeof(char[KEY_SIZE]) * lda->v);
    memset(lda->id_v_map, 0, sizeof(char[KEY_SIZE]) * lda->v);
    lda->tokens = (int (*)[3]) malloc(sizeof(int[3]) * lda->t);
    memset(lda->tokens, 0, sizeof(int[3]) * lda->t);
    lda->xy = (double (*)[2])malloc(sizeof(double[2]) * lda->t);
    memset(lda->xy, 0, sizeof(double[2]) * lda->t);
    lda->nd = (int*) malloc(sizeof(int) * lda->d * (lda->p.k));
    memset(lda->nd, 0, sizeof(int) * lda->d * (lda->p.k));
    lda->nw = (int*) malloc(sizeof(int) * lda->v * (lda->p.k));
    memset(lda->nw, 0, sizeof(int) * lda->v * (lda->p.k));
    lda->nkw = (int *) malloc(sizeof(int) * (lda->p.k));
    memset(lda->nkw, 0, sizeof(int) * (lda->p.k));
    lda->disp = (double(*)[11])malloc(sizeof(double[11]) * lda->p.k);
    memset(lda->disp, 0, sizeof(double[11]) * lda->p.k);
}

static void update_g_param(Lda * lda){
    double mx, my, xx, yy, xy, det, invxx, invyy, invxy;
    int k = 0;
    for (k = 0; k < lda->p.k; k++){
        mx = lda->disp[k][0] / lda->nkw[k];
        my = lda->disp[k][1] / lda->nkw[k];
        xx = lda->disp[k][2] / lda->nkw[k] - mx * mx;
        yy = lda->disp[k][3] / lda->nkw[k] - my * my;
        xy = lda->disp[k][4] / lda->nkw[k] - mx * my;
        det = xx * yy - xy * xy;
        invxx = yy / det;
        invyy = xx / det;
        invxy = -xy / det;
        lda->disp[k][5] = mx;
        lda->disp[k][6] = my;
        lda->disp[k][7] = det;
        lda->disp[k][8] = invxx;
        lda->disp[k][9] = invyy;
        lda->disp[k][10] = invxy;
    }
}

static void gibbs_sample(Lda * lda){
    int st, tkid, uid, vid, tid, k;
    double *prob = (double*)malloc(sizeof(double) * lda->p.k);
    double vb = lda->p.b * lda->v, rnd, x, y;
    for (tkid = 0; tkid < lda->t; tkid++){
        uid = lda->tokens[tkid][0];
        vid = lda->tokens[tkid][1];
        tid = lda->tokens[tkid][2];
        x   = lda->xy[tkid][0];
        y   = lda->xy[tkid][1];
        lda->nd[uid * lda->p.k + tid] -= 1;
        lda->nw[vid * lda->p.k + tid] -= 1;
        lda->nkw[tid] -= 1;
        lda->disp[tid][0] -= x;
        lda->disp[tid][1] -= y;
        lda->disp[tid][2] -= x * x;
        lda->disp[tid][3] -= y * y;
        lda->disp[tid][4] -= x * y;
        for (k = 0; k < lda->p.k; k++){
            prob[k] = 1.0 * (lda->nd[uid * lda->p.k + k] + lda->p.a) * \
                            (lda->nw[vid * lda->p.k + k] + lda->p.b) / \
                            (lda->nkw[k] + vb);
            /*
            double mx = lda->disp[k][0] / lda->nkw[k];
            double my = lda->disp[k][1] / lda->nkw[k];
            double xx = lda->disp[k][2] / lda->nkw[k] - mx * mx;
            double yy = lda->disp[k][3] / lda->nkw[k] - my * my;
            double xy = lda->disp[k][4] / lda->nkw[k] - mx * my;
            double det = xx * yy - xy * xy;
            double invxx = yy / det;
            double invyy = xx / det;
            double invxy = -xy / det;
            */
            double dx    = x - lda->disp[k][5];
            double dy    = y - lda->disp[k][6];
            double det   = lda->disp[k][7];
            double invxx = lda->disp[k][8];
            double invyy = lda->disp[k][9];
            double invxy = lda->disp[k][10];
            prob[k] *= 1.0 / (sqrt(det) * exp(0.5 * (dx * dx * invxx + dy * dy * invyy + 2.0 * dx * dy * invxy)));
            if (k > 0){
                prob[k] += prob[k - 1];
            }
        }
        rnd = prob[lda->p.k - 1] * rand() / (1.0 + RAND_MAX);
        for (st = 0; st < lda->p.k; st++){
            if (prob[st] > rnd) break;
        }
        if (st == lda->p.k){
            fprintf(stderr, "sample failed, instance : %d\n", tkid);
            st = tid;
        }
        lda->nd[uid * lda->p.k + st] += 1;
        lda->nw[vid * lda->p.k + st] += 1;
        lda->nkw[st] += 1;
        lda->disp[st][0] += x;
        lda->disp[st][1] += y;
        lda->disp[st][2] += x * x;
        lda->disp[st][3] += y * y;
        lda->disp[st][4] += x * y;
        lda->tokens[tkid][2] = st;
    }
    free(prob); prob = NULL;
}

Lda *create_lda(ParamLda p){
    Lda * lda = (Lda*)malloc(sizeof(Lda));
    memset(lda, 0, sizeof(Lda));
    lda->p = p;
    return lda;
}

int init_lda(Lda * lda){
    FILE * fp = NULL;
    if(NULL == (fp = fopen(lda->p.in_file, "r"))){
        fprintf(stderr, "can not open file \"%s\"\n", lda->p.in_file);
        return -1;
    }
    char buffer[LDA_LINE_LEN] = {'\0'};
    char *string, *token;
    int token_size = 0;
    int uid = -1, vid = -1, tid = -1, tkid = 0;
    double x, y;
    Hash * uhs = hash_create(1<<20, STRING);
    Hash * vhs = hash_create(1<<20, STRING);
    while(NULL != fgets(buffer, LDA_LINE_LEN, fp)) {
        string = trim(buffer, 3);
        token = strsep(&string, "\t");
        hash_add(uhs, token);
        token = strsep(&string, "\t");
        hash_add(vhs, token);
        token_size += 1;
    }
    lda->d = hash_cnt(uhs);
    lda->v = hash_cnt(vhs);
    lda->t = token_size;
    malloc_space(lda);
    rewind(fp);
    while (NULL != fgets(buffer, LDA_LINE_LEN, fp)) {
        string = trim(buffer, 3);
        token = strsep(&string, "\t");
        uid = hash_find(uhs, token);
        if (lda->id_doc_map[uid][0] == '\0'){
            strncpy(lda->id_doc_map[uid], token, KEY_SIZE - 1);
        }
        lda->tokens[tkid][0] = uid;
        token = strsep(&string, "\t");
        vid = hash_find(vhs, token);
        if (lda->id_v_map[vid][0] == '\0'){
            strncpy(lda->id_v_map[vid], token, KEY_SIZE - 1);
        }
        lda->tokens[tkid][1] = vid;
        token = strsep(&string, "\t");
        lda->xy[tkid][0] = atof(token);
        token = strsep(&string, "\t");
        lda->xy[tkid][1] = atof(token);
        tid = (int) ((0.1 + rand()) / (0.1 + RAND_MAX) * (lda->p.k));
        token = strsep(&string, "\t");
        if (token){
            tid = atoi(token);
        }
        lda->tokens[tkid][2] = tid;
        tkid += 1;
    }
    fclose(fp);
    hash_free(uhs); uhs = NULL;
    hash_free(vhs); vhs = NULL;
    for (tkid = 0; tkid < lda->t; tkid++){
        uid = lda->tokens[tkid][0];
        vid = lda->tokens[tkid][1];
        tid = lda->tokens[tkid][2];
        x   = lda->xy[tkid][0];
        y   = lda->xy[tkid][1];
        lda->nd[uid * lda->p.k + tid] += 1;
        lda->nw[vid * lda->p.k + tid] += 1;
        lda->nkw[tid] += 1;
        lda->disp[tid][0] += x;
        lda->disp[tid][1] += y;
        lda->disp[tid][2] += x * x;
        lda->disp[tid][3] += y * y;
        lda->disp[tid][4] += x * y;
    }
    update_g_param(lda);
    return 0;
}

void est_lda(Lda * lda){
    save_lda(lda, 0);
    for (int n = 1; n <= lda->p.niters; n++){
        long sec1 = time(NULL);
        gibbs_sample(lda);
        update_g_param(lda);
        long sec2 = time(NULL);
        fprintf(stderr, "iter %d done, using %ld seconds\n", n, sec2 - sec1);
        if (n % lda->p.savestep == 0){
            save_lda(lda, n);
        }
    }
}

void save_lda(Lda * lda, int n){
    FILE * fp = NULL;
    char nd_file[512] = {'\0'};
    char nw_file[512] = {'\0'};
    char tk_file[512] = {'\0'};
    char ps_file[512] = {'\0'};
    if (n < lda->p.niters){
        sprintf(nd_file,  "%s/%d_doc_topic",  lda->p.out_dir, n);
        sprintf(nw_file,  "%s/%d_word_topic", lda->p.out_dir, n);
        sprintf(tk_file,  "%s/%d_token_topic",lda->p.out_dir, n);
        sprintf(ps_file,  "%s/%d_topic_xy"   ,lda->p.out_dir, n);
    }
    else{
        sprintf(nd_file,  "%s/%s_doc_topic",  lda->p.out_dir, "f");
        sprintf(nw_file,  "%s/%s_word_topic", lda->p.out_dir, "f");
        sprintf(tk_file,  "%s/%s_token_topic",lda->p.out_dir, "f");
        sprintf(ps_file,  "%s/%s_topic_xy"   ,lda->p.out_dir, "f");
    }
    if (NULL == (fp = fopen(nd_file, "w"))) {
        fprintf(stderr, "can not open file \"%s\"\n", nd_file);
        return;
    }
    for (int d = 0; d < lda->d; d++){
        fprintf(fp, "%s", lda->id_doc_map[d]);
        int offs = d * lda->p.k;
        for (int k = 0; k < lda->p.k; k++){
            fprintf(fp, "\t%d", lda->nd[offs + k]);
        }
        fprintf(fp, "\n");
    }
    fclose(fp);
    if (NULL == (fp = fopen(nw_file, "w"))) {
        fprintf(stderr, "can not open file \"%s\"\n", nw_file);
        return;
    }
    for (int v = 0; v < lda->v; v++){
        fprintf(fp, "%s", lda->id_v_map[v]);
        int offs = v * lda->p.k;
        for (int k = 0; k < lda->p.k; k++){
            fprintf(fp, "\t%d", lda->nw[offs + k]);
        }
        fprintf(fp, "\n");
    }
    fclose(fp);
    if (NULL == (fp = fopen(tk_file, "w"))) {
        fprintf(stderr, "can not open file \"%s\"", tk_file);
        return;
    }
    for (int t = 0; t < lda->t; t++) {
        fprintf(fp, "%s\t%s\t%.4f\t%.4f\t%d\n",   lda->id_doc_map[lda->tokens[t][0]],  \
                                                  lda->id_v_map[lda->tokens[t][1]],    \
                                                  lda->xy[t][0], lda->xy[t][1],        \
                                                  lda->tokens[t][2]);
    }
    fclose(fp);
    if (NULL == (fp = fopen(ps_file, "w"))) {
        fprintf(stderr, "can not open file \"%s\"\n", ps_file);
        return;
    }
    for (int k = 0; k < lda->p.k; k++){
        fprintf(fp, "%.8f\t%.8f\t%.8f\t%.8f\t%.8f\n", lda->disp[k][0], lda->disp[k][1], \
                                     lda->disp[k][2], lda->disp[k][3], lda->disp[k][4]);
    }
    fclose(fp);
}

void free_lda(Lda *lda) {
    if (lda) {
        if (lda->id_doc_map) {
            free(lda->id_doc_map);
            lda->id_doc_map = NULL;
        }
        if (lda->id_v_map) {
            free(lda->id_v_map);
            lda->id_v_map = NULL;
        }
        if (lda->tokens){
            free(lda->tokens);
            lda->tokens = NULL;
        }
        if (lda->xy){
            free(lda->xy);
            lda->xy = NULL;
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
        if (lda->disp){
            free(lda->disp);
            lda->disp = NULL;
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
