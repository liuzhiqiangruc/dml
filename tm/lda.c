/* ========================================================
 *   Copyright (C) 2015 All rights reserved.
 *   
 *   filename : lda.c
 *   author   : liuzhiqiang01@baidu.com
 *   date     : 2015-03-26
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

static void malloc_space(Lda *lda) {
    // id_doc_map
    lda->id_doc_map = (char (*)[KEY_SIZE]) malloc(sizeof(char[KEY_SIZE]) * lda->d);
    memset(lda->id_doc_map, 0, sizeof(char[KEY_SIZE]) * lda->d);
    // doc entry
    lda->doc_entry = (int*)malloc(sizeof(int) * lda->d);
    memset(lda->doc_entry, -1, sizeof(int) * lda->d);
    // id_word_map
    lda->id_v_map = (char (*)[KEY_SIZE]) malloc(sizeof(char[KEY_SIZE]) * lda->v);
    memset(lda->id_v_map, 0, sizeof(char[KEY_SIZE]) * lda->v);
    // tokens
    lda->tokens = (int (*)[4]) malloc(sizeof(int[4]) * lda->t);
    memset(lda->tokens, 0, sizeof(int[4]) * lda->t);
    // theta matrix
    lda->nd = (ModelEle *) malloc(sizeof(ModelEle) * lda->d * (lda->p.k + 1));
    memset(lda->nd, 0, sizeof(ModelEle) * lda->d * (lda->p.k + 1));
    // phi matrix
    lda->nw = (ModelEle *) malloc(sizeof(ModelEle) * lda->v * (lda->p.k + 1));
    memset(lda->nw, 0, sizeof(ModelEle) * lda->v * (lda->p.k + 1));
    // topic token count
    lda->nkw = (int *) malloc(sizeof(int) * (lda->p.k + 1));
    memset(lda->nkw, 0, sizeof(int) * (lda->p.k + 1));
}

static void fullfill_param(Lda *lda) {
    for (int i = 0; i < lda->t; i++) {
        int uid = lda->tokens[i][0];
        int vid = lda->tokens[i][1];
        int tid = lda->tokens[i][2];
        lda->nd[uid * (lda->p.k + 1) + tid].count += 1;
        lda->nw[vid * (lda->p.k + 1) + tid].count += 1;
        lda->nkw[tid] += 1;
    }
    for (int d = 0; d < lda->d; d++){
        int offs = d * (lda->p.k + 1);
        int p = 0;
        for (int k = 1; k <= lda->p.k; k++){
            if (lda->nd[offs + k].count > 0){
                lda->nd[offs + p].next = k;
                lda->nd[offs + k].prev = p;
                p = k;
            }
        }
        lda->nd[offs + p].next = 0;
        lda->nd[offs].prev = p;
    }
    for (int v = 0; v < lda->v; v++){
        int offs = v * (lda->p.k + 1);
        int p = 0;
        for (int k = 1; k <= lda->p.k; k++){
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

static int gibbs_sample(Lda * lda) {
    double ab = lda->p.a * lda->p.b;
    double vb = lda->v * lda->p.b;
    double e, f, g, tmp, u;
    int k, d, id, vid, tid, offs, voffs;
    // calculate smooth bucket value e
    // for all documents 
    // recalculate for each iteration
    e = 0.0;
    for (k = 1; k <= lda->p.k; k++){
        e += ab / (vb + lda->nkw[k]);
    }
    // scan the d documents one by one
    for (d = 0; d < lda->d; d++){
        // doc topic bucket value f for document d
        f = 0.0;
        offs = d * (lda->p.k + 1);
        k = lda->nd[offs].next;
        while (k != 0){
            f += lda->nd[offs + k].count * lda->p.b / (vb + lda->nkw[k]);
            k = lda->nd[offs + k].next;
        }
        // scan the tokens in document d
        id = lda->doc_entry[d];
        while (id != -1){
            vid = lda->tokens[id][1];
            tid = lda->tokens[id][2];
            voffs = vid * (lda->p.k + 1);
            lda->nd[offs  + tid].count -= 1;
            lda->nw[voffs + tid].count -= 1;
            lda->nkw[tid] -= 1;
            if (lda->nd[offs + tid].count == 0){
                lda->nd[offs + lda->nd[offs + tid].prev].next = lda->nd[offs + tid].next;
                lda->nd[offs + lda->nd[offs + tid].next].prev = lda->nd[offs + tid].prev;
            }
            if (lda->nw[voffs + tid].count == 0){
                lda->nw[voffs + lda->nw[voffs + tid].prev].next = lda->nw[voffs + tid].next;
                lda->nw[voffs + lda->nw[voffs + tid].next].prev = lda->nw[voffs + tid].prev;
            }
            tmp = (vb + lda->nkw[tid]) * (vb + lda->nkw[tid] + 1);
            e += ab / tmp;
            f += lda->p.b * (lda->nd[offs + tid].count - vb - lda->nkw[tid]) / tmp;
            g = 0.0;
            k = lda->nw[voffs].next;
            while (k != 0){
                g += lda->nw[voffs + k].count * (lda->p.a + lda->nd[offs + k].count) / (vb + lda->nkw[k]);
                k = lda->nw[voffs + k].next;
            }
            u = (e + f + g) * (0.1 + rand()) / (1.0 + RAND_MAX);
            // sample k from smooth bucket
            if (u < e){
                double s = 0.0;
                for (k = 1; k <= lda->p.k; k++){
                    s += ab / (vb + lda->nkw[k]);
                    if (s > u){
                        break;
                    }
                }
            }
            // sample k from doc topic bucket
            else if (u < e + f) {
                u -= e;
                double s = 0.0;
                k = lda->nd[offs].next;
                while (k != 0){
                    s += 1.0 * lda->nd[offs + k].count * lda->p.b / (vb + lda->nkw[k]);
                    if (s > u){
                        break;
                    }
                    k = lda->nd[offs + k].next;
                }
            }
            // sample k from topic word bucket
            else {
                u -= (e + f);
                double s = 0.0;
                k = lda->nw[voffs].next;
                while (k != 0){
                    s += 1.0 * lda->nw[voffs + k].count * (lda->p.a + lda->nd[offs + k].count) / (vb + lda->nkw[k]);
                    if (s > u){
                        break;
                    }
                    k = lda->nw[voffs + k].next;
                }
            }
            // sample failed
            if (k == 0){
                fprintf(stderr, "gibbs sample failed\n");
                return -1;
            }
            // link a new topic into theta matrix
            if (lda->nd[offs + k].count == 0){
                lda->nd[offs + lda->nd[offs].prev].next = k;
                lda->nd[offs + k].prev = lda->nd[offs].prev;
                lda->nd[offs + k].next = 0;
                lda->nd[offs].prev = k;
            }
            lda->nd[offs + k].count += 1;
            // link a new topic into phi matrix
            if (lda->nw[voffs + k].count == 0){
                lda->nw[voffs + lda->nw[voffs].prev].next = k;
                lda->nw[voffs + k].prev = lda->nw[voffs].prev;
                lda->nw[voffs + k].next = 0;
                lda->nw[voffs].prev = k;
            }
            lda->nw[voffs + k].count += 1;
            lda->nkw[k] += 1;
            lda->tokens[id][2] = k;
            // update the smooth bucket e value
            // and doc topic bucket f value
            tmp = (vb + lda->nkw[k]) * (vb + lda->nkw[k] - 1);
            e -= ab / tmp;
            f += lda->p.b * (vb + lda->nkw[k] - lda->nd[offs + k].count) / tmp;
            // next token id of document d
            id = lda->tokens[id][3];
        }
    }
    return 0;
}

Lda *create_lda() {
    Lda *lda = (Lda *) malloc(sizeof(Lda));
    memset(lda, 0, sizeof(Lda));
    return lda;
}

int init_lda(Lda *lda) {
    FILE *fp = NULL;
    if (NULL == (fp = fopen(lda->p.in_file, "r"))) {
        fprintf(stderr, "can not open file \"%s\"\n", lda->p.in_file);
        return -1;
    }
    char buffer[LDA_LINE_LEN] = {'\0'};
    char *string = NULL, *token = NULL;
    int token_size = 0;
    Hash * uhs = hash_create(1<<20, STRING);
    Hash * vhs = hash_create(1<<20, STRING);
    // first scan to generate uniq doc & word set
    while (NULL != fgets(buffer, LDA_LINE_LEN, fp)) {
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
    // malloc the model space for iteration
    malloc_space(lda);
    // re scan the input file to load data
    rewind(fp);
    int uid = -1, vid = -1, tid = -1;
    int token_index = 0;
    while (NULL != fgets(buffer, LDA_LINE_LEN, fp)) {
        string = trim(buffer, 3);
        // read doc
        token = strsep(&string, "\t");
        uid = hash_find(uhs, token);
        if (lda->id_doc_map[uid][0] == '\0'){
            strncpy(lda->id_doc_map[uid], token, KEY_SIZE - 1);
        }
        lda->tokens[token_index][0] = uid;
        // read word
        token = strsep(&string, "\t");
        vid = hash_find(vhs, token);
        if (lda->id_v_map[vid][0] == '\0'){
            strncpy(lda->id_v_map[vid], token, KEY_SIZE - 1);
        }
        lda->tokens[token_index][1] = vid;
        // read tid
        tid = (int) ((0.1 + rand()) / (0.1 + RAND_MAX) * (lda->p.k));
        tid += 1;
        token = strsep(&string, "\t");
        if (token){
            tid = atoi(token);
        }
        lda->tokens[token_index][2] = tid;
        // link the tokens for current doc
        lda->tokens[token_index][3] = lda->doc_entry[uid];
        lda->doc_entry[uid] = token_index;
        // token_index ++
        token_index += 1;
    }
    fclose(fp);
    hash_free(uhs);    uhs = NULL;
    hash_free(vhs);    vhs = NULL;
    return 0;
}

void est_lda(Lda *lda) {
    // first full fill theta & phi matrix
    // and link the nonzero elements
    fullfill_param(lda);
    // est iteration for lda
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
        int offs = d * (lda->p.k + 1);
        for (int k = 1; k <= lda->p.k; k++) {
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
    for (int v = 0; v < lda->v; v++) {
        fprintf(fp, "%s", lda->id_v_map[v]);
        int offs = v * (lda->p.k + 1);
        for (int k = 1; k <= lda->p.k; k++) {
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
        if (lda->doc_entry){
            free(lda->doc_entry);
            lda->doc_entry = NULL;
        }
        free(lda);
    }
}
