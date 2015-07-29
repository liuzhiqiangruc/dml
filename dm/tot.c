/* ========================================================
 *   Copyright (C) 2015 All rights reserved.
 *   
 *   filename : tot.c
 *   author   : liuzhiqiang01@baidu.com
 *   date     : 2015-04-27
 *   info     : implementation of topics over time
 * ======================================================== */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "idmap.h"
#include "str.h"
#include "tot.h"


#define TOT_LINE_LEN 256
#define DOUBLE_ESP   0.000001
#define MIN_VAR      0.0001

TOT * create_tot(){
    TOT * tot = (TOT*)malloc(sizeof(TOT));
    memset(tot, 0, sizeof(TOT));
    return tot;
}

static void malloc_space(TOT *tot) {
    tot->id_doc_map = (char (*)[KEY_SIZE]) malloc(sizeof(char[KEY_SIZE]) * tot->d);
    memset(tot->id_doc_map, 0, sizeof(char[KEY_SIZE]) * tot->d);

    tot->id_v_map = (char (*)[KEY_SIZE]) malloc(sizeof(char[KEY_SIZE]) * tot->v);
    memset(tot->id_v_map, 0, sizeof(char[KEY_SIZE]) * tot->v);

    tot->tokens = (int (*)[4]) malloc(sizeof(int[4]) * tot->t);
    memset(tot->tokens, 0, sizeof(int[4]) * tot->t);

    tot->tk_time = (double*)malloc(sizeof(double) * tot->t);
    memset(tot->tk_time, 0, sizeof(double) * tot->t);

    tot->nd = (int *) malloc(sizeof(int) * tot->d * tot->p.k);
    memset(tot->nd, 0, sizeof(int) * tot->d * tot->p.k);

    tot->nw = (int *) malloc(sizeof(int) * tot->v * tot->p.k);
    memset(tot->nw, 0, sizeof(int) * tot->v * tot->p.k);

    tot->nkw = (int *) malloc(sizeof(int) * tot->p.k);
    memset(tot->nkw, 0, sizeof(int) * tot->p.k);

    tot->beta = (double(*)[5]) malloc(sizeof(double[5]) * tot->p.k);
    memset(tot->beta, 0, sizeof(double[5]) * tot->p.k);
}

static void update_beta_p(TOT *tot){
    double mean = 0.0, var = 0.0, cm = 0.0;
    for (int k = 0; k < tot->p.k; k++){
        mean = 1.0 * tot->beta[k][0] / tot->nkw[k];
        var  = 1.0 * tot->beta[k][1] / tot->nkw[k] \
             - 1.0 * tot->beta[k][0] * tot->beta[k][0] / tot->nkw[k] / tot->nkw[k];
        if (var < DOUBLE_ESP){
            var = 0.0;
        }
        var += MIN_VAR;
        cm = mean * (1.0 - mean) / var - 1.0;
        if (cm < 0.0) cm *= -1.0;
        tot->beta[k][2] = mean * cm;
        tot->beta[k][3] = (1.0 - mean) * cm;
        tot->beta[k][4] = lgamma(tot->beta[k][2]) + lgamma(tot->beta[k][3]) \
                        - lgamma(tot->beta[k][2]  + tot->beta[k][3]);
    }
}

static void fullfill_param(TOT *tot) {
    for (int i = 0; i < tot->t; i++) {
        int uid = tot->tokens[i][0];
        int vid = tot->tokens[i][1];
        int top = tot->tokens[i][3];
        double tm = tot->tk_time[i];

        tot->nd[uid * tot->p.k + top] += 1;
        tot->nw[vid * tot->p.k + top] += 1;
        tot->nkw[top] += 1;

        tot->beta[top][0] += tm;
        tot->beta[top][1] += tm * tm;
    }
#ifdef BETA
    update_beta_p(tot);
#elif  GAUSSIAN
    for (int k = 0; k < tot->p.k; k++){
         tot->beta[k][2] = tot->beta[k][0] / tot->nkw[k];
         tot->beta[k][3] = tot->beta[k][1] / tot->nkw[k] - tot->beta[k][2] * tot->beta[k][2];
         if (tot->beta[k][3] <= 0.0){
             tot->beta[k][3] = 0.0001;
         }
    }
#endif
    save_tot(tot, 0);
}

int  init_tot(TOT * tot){
    FILE *fp = NULL;
    if (NULL == (fp = fopen(tot->p.in_file, "r"))) {
        fprintf(stderr, "can not open file \"%s\"\n", tot->p.in_file);
        return -1;
    }
    char buffer[TOT_LINE_LEN];
    char **str_array = NULL;
    int count = 0, token_size = 0, min_time = 1e5, max_time = 0, tk_t = 0, time_span = 0;

    IdMap *uidmap = idmap_create();
    IdMap *vidmap = idmap_create();

    while (NULL != fgets(buffer, TOT_LINE_LEN, fp)) {
        str_array = split(trim(buffer, 3), '\t', &count);
        if (count < 3) {
            goto free_str;
        }

        if (-1 == idmap_get_value(uidmap, str_array[0])) {
            idmap_add(uidmap, dupstr(str_array[0]), idmap_size(uidmap));
        }

        if (-1 == idmap_get_value(vidmap, str_array[1])) {
            idmap_add(vidmap, dupstr(str_array[1]), idmap_size(vidmap));
        }

        tk_t = atoi(str_array[2]);
        if (tk_t < min_time) min_time = tk_t;
        if (tk_t > max_time) max_time = tk_t;

        token_size += 1;

free_str:
        free(str_array[0]);
        free(str_array);
    }

    tot->d = idmap_size(uidmap);
    tot->t = token_size;
    tot->v = idmap_size(vidmap);
    time_span = max_time - min_time;

    malloc_space(tot);

    rewind(fp);
    int uid = -1, vid = -1, tid = -1;
    int token_index = 0;
    double tk_norm = 0.0;

    while (NULL != fgets(buffer, TOT_LINE_LEN, fp)) {
        str_array = split(trim(buffer, 3), '\t', &count);
        if (count < 3) {
            goto str_free;
        }

        uid = idmap_get_value(uidmap, str_array[0]);
        strncpy(tot->id_doc_map[uid], str_array[0], KEY_SIZE - 1);
        tot->tokens[token_index][0] = uid;

        vid = idmap_get_value(vidmap, str_array[1]);
        strncpy(tot->id_v_map[vid], str_array[1], KEY_SIZE - 1);
        tot->tokens[token_index][1] = vid;

        tk_t = atoi(str_array[2]);
        tot->tokens[token_index][2] = tk_t;

#ifdef BETA
        tk_norm = 1.0 * (tk_t - min_time) / time_span;
        if (tk_norm < DOUBLE_ESP) tk_norm = DOUBLE_ESP;
        if (tk_norm > (1.0 - DOUBLE_ESP))
            tk_norm = 1.0 - DOUBLE_ESP;
#elif  GAUSSIAN
        tk_norm = atof(str_array[2]);
#endif
        tot->tk_time[token_index] = tk_norm;

        tid = (int) (1.0 * rand() / (1.0 + RAND_MAX) * (tot->p.k));
        if (count == 4){
            tid = atoi(str_array[3]);
        }

        tot->tokens[token_index][3] = tid;
        token_index += 1;

str_free:
        free(str_array[0]);
        free(str_array);
    }
    fclose(fp);

    idmap_free(uidmap);   uidmap = NULL;
    idmap_free(vidmap);   vidmap = NULL;

    return 0;

}

static void shuffle(int *a, int n) {
    int *tmp = a;
    int i, t, k;
    i = t = 0;
    for (k = n; k >= 2; k--) {
        i = rand() % k;
        t = tmp[i];
        tmp[i] = tmp[0];
        *tmp++ = t;
    }
}

void  est_tot(TOT * tot){

    fullfill_param(tot);

    int *p = (int *) malloc(sizeof(int) * tot->t);
    int st = 0;

    double *prob = (double *) malloc(sizeof(double) * tot->p.k);
    double vb = tot->p.b * tot->v;
    double rnd = 0.0;

    for (int i = 0; i < tot->t; i++) p[i] = i;
    for (int i = 1; i <= tot->p.niters; i++) {
        fprintf(stderr, "iteration %d estimate begin ... ", i);
        shuffle(p, tot->t);
        for (int j = 0; j < tot->t; j++) {
            int id = p[j];
            int uid = tot->tokens[id][0];
            int vid = tot->tokens[id][1];
            int top = tot->tokens[id][3];
            double tm = tot->tk_time[id];

            tot->nd[uid * tot->p.k + top] -= 1;
            tot->nw[vid * tot->p.k + top] -= 1;
            tot->nkw[top] -= 1;
            tot->beta[top][0] -= tm;
            tot->beta[top][1] -= tm * tm;

#ifdef GAUSSIAN
            tot->beta[top][2] = tot->beta[top][0] / tot->nkw[top];
            tot->beta[top][3] = tot->beta[top][1] / tot->nkw[top] - tot->beta[top][2] * tot->beta[top][2];
            if (tot->beta[top][3] <= 0.0){
                tot->beta[top][3] = 0.0001;
            }
#endif
            for (int l = 0; l < tot->p.k; l++) {
                prob[l] = 1.0 * (tot->nd[uid * tot->p.k + l] + tot->p.a) *
                                (tot->nw[vid * tot->p.k + l] + tot->p.b) /
                                (tot->nkw[l] + vb);
#ifdef BETA
                prob[l] *= exp((tot->beta[l][2] - 1) * log(1 - tm) + \
                               (tot->beta[l][3] - 1) * log(tm)     - \
                                tot->beta[l][4]);
#elif GAUSSIAN
                prob[l] *= exp((tm - tot->beta[l][2]) * (tot->beta[l][2] - tm) / 2.0 / tot->beta[l][3]) / sqrt(tot->beta[l][3]);
#endif 
                if (l > 0) prob[l] += prob[l - 1];
            }

            rnd = prob[tot->p.k - 1] * rand() / (1.0 + RAND_MAX);

            for (st = 0; st < tot->p.k; st++) {
                if (prob[st] > rnd) break;
            }
            if (st == tot->p.k){
                fprintf(stderr, "sample failed, instnce : %d\n", id);
                st = top;
            }

            tot->nd[uid * tot->p.k + st] += 1;
            tot->nw[vid * tot->p.k + st] += 1;
            tot->nkw[st] += 1;
            tot->beta[st][0] += tm;
            tot->beta[st][1] += tm * tm;

#ifdef GAUSSIAN
            tot->beta[st][2] = tot->beta[st][0] / tot->nkw[st];
            tot->beta[st][3] = tot->beta[st][1] / tot->nkw[st] - tot->beta[st][2] * tot->beta[st][2];
            if (tot->beta[st][3] <= 0.0){
                tot->beta[st][3] = 0.0001;
            }
#endif
            tot->tokens[id][3] = st;
        }

#ifdef BETA
        update_beta_p(tot);
#endif

        fprintf(stderr, " done\n");
        if (i % tot->p.savestep == 0) {
            save_tot(tot, i);
        }
    }
    free(p);    p    = NULL;
    free(prob); prob = NULL;

}
void save_tot(TOT * tot,int n){
    FILE *fp = NULL;
    char nd_file[512];
    char nw_file[512];
    char tk_file[512];
    char tt_file[512];

    if (n < tot->p.niters){
        sprintf(nd_file,  "%s/%d_doc_topic",  tot->p.out_dir, n);
        sprintf(nw_file,  "%s/%d_word_topic", tot->p.out_dir, n);
        sprintf(tk_file,  "%s/%d_token_topic",tot->p.out_dir, n);
        sprintf(tt_file,  "%s/%d_topic_time", tot->p.out_dir, n);
    }
    else{
        sprintf(nd_file,  "%s/%s_doc_topic",  tot->p.out_dir, "f");
        sprintf(nw_file,  "%s/%s_word_topic", tot->p.out_dir, "f");
        sprintf(tk_file,  "%s/%s_token_topic",tot->p.out_dir, "f");
        sprintf(tt_file,  "%s/%s_topic_time", tot->p.out_dir, "f");
    }

    //output for nd
    if (NULL == (fp = fopen(nd_file, "w"))) {
        fprintf(stderr, "can not open file \"%s\"", nd_file);
        return;
    }
    for (int d = 0; d < tot->d; d++) {
        fprintf(fp, "%s", tot->id_doc_map[d]);
        for (int k = 0; k < tot->p.k; k++) {
            fprintf(fp, "\t%d", tot->nd[d * tot->p.k + k]);
        }
        fprintf(fp, "\n");
    }
    fclose(fp);

    //output for nw
    if (NULL == (fp = fopen(nw_file, "w"))) {
        fprintf(stderr, "can not open file \"%s\"", nw_file);
        return;
    }
    for (int v = 0; v < tot->v; v++) {
        fprintf(fp, "%s", tot->id_v_map[v]);
        for (int k = 0; k < tot->p.k; k++) {
            fprintf(fp, "\t%d", tot->nw[v * tot->p.k + k]);
        }
        fprintf(fp, "\n");
    }
    fclose(fp);

    //output for tk
    if (NULL == (fp = fopen(tk_file, "w"))) {
        fprintf(stderr, "can not open file \"%s\"", tk_file);
        return;
    }
    for (int t = 0; t < tot->t; t++) {
        fprintf(fp, "%s\t%s\t%d\t%d\n", tot->id_doc_map[tot->tokens[t][0]],  \
                                        tot->id_v_map[tot->tokens[t][1]],    \
                                        tot->tokens[t][2], tot->tokens[t][3]);
    }
    fclose(fp);

    //output for tt
    if (NULL == (fp = fopen(tt_file, "w"))) {
        fprintf(stderr, "can not open file \"%s\"", tt_file);
        return;
    }
    for (int k = 0; k < tot->p.k; k++) {
        fprintf(fp, "%e\t%e\t%e\t%e\t%e\n", tot->beta[k][0],    \
                                            tot->beta[k][1],    \
                                            tot->beta[k][2],    \
                                            tot->beta[k][3],    \
                                            exp(tot->beta[k][4]));
    }
    fclose(fp);
}

void free_tot(TOT * tot) {
    if (tot) {
        if (tot->id_doc_map) {
            free(tot->id_doc_map);
            tot->id_doc_map = NULL;
        }
        if (tot->id_v_map) {
            free(tot->id_v_map);
            tot->id_v_map = NULL;
        }
        if (tot->tokens){
            free(tot->tokens);
            tot->tokens = NULL;
        }
        if (tot->nd) {
            free(tot->nd);
            tot->nd = NULL;
        }
        if (tot->nw) {
            free(tot->nw);
            tot->nw = NULL;
        }
        if (tot->nkw) {
            free(tot->nkw);
            tot->nkw = NULL;
        }
        if (tot->beta) {
            free(tot->beta);
            tot->beta = NULL;
        }
        if (tot->tk_time) {
            free(tot->tk_time);
            tot->tk_time = NULL;
        }
        free(tot);
    }
}
