/* ========================================================
 *   Copyright (C) 2015 All rights reserved.
 *   
 *   filename : lda.c
 *   author   : liuzhiqiang01@baidu.com
 *   date     : 2015-03-26
 *   info     :
 * ======================================================== */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "str.h"
#include "lda.h"
#include "idmap.h"

#define LDA_LINE_LEN 1024

Lda *create_lda() {
    Lda *lda = (Lda *) malloc(sizeof(Lda));
    memset(lda, 0, sizeof(Lda));
    return lda;
}

static void malloc_space(Lda *lda) {
    lda->id_doc_map = (char (*)[KEY_SIZE]) malloc(sizeof(char[KEY_SIZE]) * lda->d);
    memset(lda->id_doc_map, 0, sizeof(char[KEY_SIZE]) * lda->d);

    lda->id_v_map = (char (*)[KEY_SIZE]) malloc(sizeof(char[KEY_SIZE]) * lda->v);
    memset(lda->id_v_map, 0, sizeof(char[KEY_SIZE]) * lda->v);

    lda->tokens = (int (*)[3]) malloc(sizeof(int[3]) * lda->t);
    memset(lda->tokens, 0, sizeof(int[3]) * lda->t);

    lda->nd = (int *) malloc(sizeof(int) * lda->d * lda->p.k);
    memset(lda->nd, 0, sizeof(int) * lda->d * lda->p.k);

    lda->nw = (int *) malloc(sizeof(int) * lda->v * lda->p.k);
    memset(lda->nw, 0, sizeof(int) * lda->v * lda->p.k);

    lda->nkw = (int *) malloc(sizeof(int) * lda->p.k);
    memset(lda->nkw, 0, sizeof(int) * lda->p.k);
}

static void fullfill_param(Lda *lda) {
    for (int i = 0; i < lda->t; i++) {
        int uid = lda->tokens[i][0];
        int vid = lda->tokens[i][1];
        int top = lda->tokens[i][2];

        lda->nd[uid * lda->p.k + top] += 1;
        lda->nw[vid * lda->p.k + top] += 1;
        lda->nkw[top] += 1;
    }
    save_lda(lda, 0);
}

int init_lda(Lda *lda) {
    FILE *fp = NULL;
    if (NULL == (fp = fopen(lda->p.in_file, "r"))) {
        fprintf(stderr, "can not open file \"%s\"\n", lda->p.in_file);
        return -1;
    }
    char buffer[LDA_LINE_LEN];
    char **str_array = NULL;
    int count = 0, token_size = 0;

    IdMap *uidmap = idmap_create();
    IdMap *vidmap = idmap_create();

    while (NULL != fgets(buffer, LDA_LINE_LEN, fp)) {
        str_array = split(trim(buffer, 3), '\t', &count);
        if (count < 2) {
            goto free_str;
        }

        if (-1 == idmap_get_value(uidmap, str_array[0])) {
            idmap_add(uidmap, dupstr(str_array[0]), idmap_size(uidmap));
        }

        if (-1 == idmap_get_value(vidmap, str_array[1])) {
            idmap_add(vidmap, dupstr(str_array[1]), idmap_size(vidmap));
        }
        token_size += 1;

free_str:
        free(str_array[0]);
        free(str_array);
    }

    lda->d = idmap_size(uidmap);
    lda->t = token_size;
    lda->v = idmap_size(vidmap);

    malloc_space(lda);

    rewind(fp);
    int uid = -1, vid = -1, tid = -1;
    int token_index = 0;

    while (NULL != fgets(buffer, LDA_LINE_LEN, fp)) {
        str_array = split(trim(buffer, 3), '\t', &count);
        if (count < 2) {
            goto str_free;
        }

        uid = idmap_get_value(uidmap, str_array[0]);
        strncpy(lda->id_doc_map[uid], str_array[0], KEY_SIZE - 1);
        lda->tokens[token_index][0] = uid;

        vid = idmap_get_value(vidmap, str_array[1]);
        strncpy(lda->id_v_map[vid], str_array[1], KEY_SIZE - 1);
        lda->tokens[token_index][1] = vid;

        tid = (int) ((1.0 + rand()) / (1.0 + RAND_MAX) * (lda->p.k));
        if (count == 3){
            tid = atoi(str_array[2]);
        }

        lda->tokens[token_index][2] = tid;
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

void est_lda(Lda *lda) {

    fullfill_param(lda);

    int *p = (int *) malloc(sizeof(int) * lda->t);
    int st = 0;

    double *prob = (double *) malloc(sizeof(double) * lda->p.k);
    double vb = lda->p.b * lda->v;

    for (int i = 0; i < lda->t; i++) p[i] = i;
    for (int i = 1; i <= lda->p.niters; i++) {
        fprintf(stderr, "iteration %d estimate begin ... ", i);
        shuffle(p, lda->t);
        for (int j = 0; j < lda->t; j++) {
            int id = p[j];
            int uid = lda->tokens[id][0];
            int vid = lda->tokens[id][1];
            int top = lda->tokens[id][2];

            lda->nd[uid * lda->p.k + top] -= 1;
            lda->nw[vid * lda->p.k + top] -= 1;
            lda->nkw[top] -= 1;

            for (int l = 0; l < lda->p.k; l++) {
                prob[l] = 1.0 * (lda->nd[uid * lda->p.k + l] + lda->p.a) *
                                (lda->nw[vid * lda->p.k + l] + lda->p.b) /
                                (lda->nkw[l] + vb);
                if (l > 0) prob[l] += prob[l - 1];
            }
            double rnd = prob[lda->p.k - 1] * (0.1 + rand()) / (0.1 + RAND_MAX);
            for (st = 0; st < lda->p.k; st++) {
                if (prob[st] > rnd) break;
            }

            lda->nd[uid * lda->p.k + st] += 1;
            lda->nw[vid * lda->p.k + st] += 1;
            lda->nkw[st] += 1;

            lda->tokens[id][2] = st;
        }

        fprintf(stderr, " done\n");
        if (i % lda->p.savestep == 0) {
            save_lda(lda, i);
        }
    }
    free(p);    p    = NULL;
    free(prob); prob = NULL;
}

void save_lda(Lda *lda, int n) {
    FILE *fp = NULL;
    char nd_file[512];
    char nw_file[512];
    char tk_file[512];

    if (n < lda->p.niters){
        sprintf(nd_file,  "%s/%d_doc_topic",  lda->p.out_dir, n);
        sprintf(nw_file,  "%s/%d_word_topic", lda->p.out_dir, n);
        sprintf(tk_file,  "%s/%d_token_topic",lda->p.out_dir, n);
    }
    else{
        sprintf(nd_file,  "%s/%s_doc_topic",  lda->p.out_dir, "f");
        sprintf(nw_file,  "%s/%s_word_topic", lda->p.out_dir, "f");
        sprintf(tk_file,  "%s/%s_token_topic",lda->p.out_dir, "f");
    }

    //output for nd
    if (NULL == (fp = fopen(nd_file, "w"))) {
        fprintf(stderr, "can not open file \"%s\"", nd_file);
        return;
    }
    for (int d = 0; d < lda->d; d++) {
        fprintf(fp, "%s", lda->id_doc_map[d]);
        for (int k = 0; k < lda->p.k; k++) {
            fprintf(fp, "\t%d", lda->nd[d * lda->p.k + k]);
        }
        fprintf(fp, "\n");
    }
    fclose(fp);

    //output for nw
    if (NULL == (fp = fopen(nw_file, "w"))) {
        fprintf(stderr, "can not open file \"%s\"", nw_file);
        return;
    }
    for (int v = 0; v < lda->v; v++) {
        fprintf(fp, "%s", lda->id_v_map[v]);
        for (int k = 0; k < lda->p.k; k++) {
            fprintf(fp, "\t%d", lda->nw[v * lda->p.k + k]);
        }
        fprintf(fp, "\n");
    }
    fclose(fp);

    //output for tk
    if (NULL == (fp = fopen(tk_file, "w"))) {
        fprintf(stderr, "can not open file \"%s\"", tk_file);
        return;
    }
    for (int t = 0; t < lda->t; t++) {
        fprintf(fp, "%s\t%s\t%d\n",   lda->id_doc_map[lda->tokens[t][0]],  \
                                      lda->id_v_map[lda->tokens[t][1]],    \
                                      lda->tokens[t][2]);
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
        free(lda);
    }
}

