/* ========================================================
 *   Copyright (C) 2015 All rights reserved.
 *   
 *   filename : bri_lda.c
 *   author   : liuzhiqiang01@baidu.com
 *   date     : 2015-04-02
 *   info     : implementation of link-lda
 * ======================================================== */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "str.h"
#include "bri_lda.h"
#include "idmap.h"

#define LDA_LINE_LEN 1024

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

static void malloc_space(BriLda *blda) {
    blda->id_doc_map = (char (*)[KEY_SIZE]) malloc(sizeof(char[KEY_SIZE]) * blda->d);
    memset(blda->id_doc_map, 0, sizeof(char[KEY_SIZE]) * blda->d);

    blda->id_v1_map = (char (*)[KEY_SIZE]) malloc(sizeof(char[KEY_SIZE]) * blda->v1);
    memset(blda->id_v1_map, 0, sizeof(char[KEY_SIZE]) * blda->v1);

    blda->id_v2_map = (char (*)[KEY_SIZE]) malloc(sizeof(char[KEY_SIZE]) * blda->v2);
    memset(blda->id_v2_map, 0, sizeof(char[KEY_SIZE]) * blda->v2);

    blda->tokens = (int (*)[4]) malloc(sizeof(int[4]) * blda->t);
    memset(blda->tokens, 0, sizeof(int[4]) * blda->t);

    blda->nd = (int *) malloc(sizeof(int) * blda->d * blda->p.k);
    memset(blda->nd, 0, sizeof(int) * blda->d * blda->p.k);

    blda->nw1 = (int *) malloc(sizeof(int) * blda->v1 * blda->p.k);
    memset(blda->nw1, 0, sizeof(int) * blda->v1 * blda->p.k);

    blda->nw2 = (int *) malloc(sizeof(int) * blda->v2 * blda->p.k);
    memset(blda->nw2, 0, sizeof(int) * blda->v2 * blda->p.k);

    blda->nkw1 = (int *) malloc(sizeof(int) * blda->p.k);
    memset(blda->nkw1, 0, sizeof(int) * blda->p.k);

    blda->nkw2 = (int *) malloc(sizeof(int) * blda->p.k);
    memset(blda->nkw2, 0, sizeof(int) * blda->p.k);
}

static void fullfill_param(BriLda * blda) {
    for (int i = 0; i < blda->t; i++) {
        int uid = blda->tokens[i][0];
        int vid = blda->tokens[i][1];
        int typ = blda->tokens[i][2];
        int top = blda->tokens[i][3];

        blda->nd[uid * blda->p.k + top] += 1;

        if (typ == 1){
            blda->nw1[vid * blda->p.k + top] += 1;
            blda->nkw1[top] += 1;
        }
        else{
            blda->nw2[vid * blda->p.k + top] += 1;
            blda->nkw2[top] += 1;
        }
    }
    save_bri_lda(blda, 0);
}

BriLda * create_bri_lda() {
    BriLda *blda = (BriLda *) malloc(sizeof(BriLda));
    memset(blda, 0, sizeof(BriLda));
    return blda;
}

int  init_bri_lda(BriLda * blda) {

    FILE * fp = NULL;

    if (NULL == (fp = fopen(blda->p.in_file,"r"))){
        fprintf(stderr,"can not open file \"%s\"\n", blda->p.in_file);
        return -1;
    }
    char buffer[LDA_LINE_LEN];
    char ** str_array = NULL;
    int count = 0, token_size = 0, type = -1;

    IdMap * uidmap  = idmap_create();
    IdMap * v1idmap = idmap_create();
    IdMap * v2idmap = idmap_create();

    while (NULL != fgets(buffer, LDA_LINE_LEN, fp)){
        str_array = split(trim(buffer,3),'\t',&count);
        if (count < 3){
            goto free_str;
        }
        type = atoi(str_array[2]);
        if (type == 1){
            if (-1 == idmap_get_value(v1idmap, str_array[1])){
                idmap_add(v1idmap, dupstr(str_array[1]), idmap_size(v1idmap));
            }
        }
        else if (type == 2){
            if (-1 == idmap_get_value(v2idmap, str_array[1])){
                idmap_add(v2idmap, dupstr(str_array[1]), idmap_size(v2idmap));
            }
        }
        else {
            goto free_str;
        }
        if (-1 == idmap_get_value(uidmap, str_array[0])){
            idmap_add(uidmap, dupstr(str_array[0]), idmap_size(uidmap));
        }
        token_size += 1;
free_str:
        free(str_array[0]);
        free(str_array);
    }

    blda->d  = idmap_size(uidmap);
    blda->t  = token_size;
    blda->v1 = idmap_size(v1idmap);
    blda->v2 = idmap_size(v2idmap);

    malloc_space(blda);

    rewind(fp);

    int uid = -1, vid = -1, tid = -1;
    int token_index = 0;

    while (NULL != fgets(buffer, LDA_LINE_LEN, fp)){
        str_array = split(trim(buffer, 3), '\t', &count);
        if (count < 3){
            goto str_free;
        }
        type = atoi(str_array[2]);
        if (type == 1){
            vid = idmap_get_value(v1idmap, str_array[1]);
            strncpy(blda->id_v1_map[vid], str_array[1], KEY_SIZE - 1);
        }
        else if (type == 2){
            vid = idmap_get_value(v2idmap, str_array[1]);
            strncpy(blda->id_v2_map[vid], str_array[1], KEY_SIZE - 1);
        }
        else{
            goto str_free;
        }

        uid = idmap_get_value(uidmap, str_array[0]);
        strncpy(blda->id_doc_map[uid], str_array[0], KEY_SIZE - 1);

        blda->tokens[token_index][0] = uid;
        blda->tokens[token_index][1] = vid;
        blda->tokens[token_index][2] = type;

        if (count == 4){
            tid = atoi(str_array[3]);
        }
        else {
            tid = (int)((1.0 + rand()) / (1.0 + RAND_MAX) * (blda->p.k));
        }
        blda->tokens[token_index][3] = tid;

        token_index += 1;
str_free:
        free(str_array[0]);
        free(str_array);
    }
    fclose(fp);

    idmap_free(uidmap);    uidmap = NULL;
    idmap_free(v1idmap);  v1idmap = NULL;
    idmap_free(v2idmap);  v2idmap = NULL;

    return 0;
}



void est_bri_lda(BriLda * blda){

    fullfill_param(blda);

    int * p = (int *)malloc(sizeof(int) * blda->t);

    double * prob = (double*)malloc(sizeof(double) * blda->p.k);
    double vb1  = blda->p.b1 * blda->v1;
    double vb2  = blda->p.b2 * blda->v2;
    int st = 0;

    for (int i = 0; i< blda->t; i++) p[i] = i;
    for (int i = 1; i <= blda->p.niters; i++){
        fprintf(stderr,"iteration %d estimate begin ... ", i);
        shuffle(p, blda->t);
        for (int j = 0; j < blda->t; j++){
            int id = p[j];
            int uid = blda->tokens[id][0];
            int vid = blda->tokens[id][1];
            int tpy = blda->tokens[id][2];
            int top = blda->tokens[id][3];
            int doff = uid * blda->p.k;
            int woff = vid * blda->p.k;
            blda->nd[doff + top] -= 1;
            if (tpy == 1){
                blda->nw1[woff + top] -= 1;
                blda->nkw1[top] -= 1;
                for (int l = 0; l < blda->p.k; l++){
                    prob[l] = 1.0 * (blda->nd[doff + l]  + blda->p.a)  * \
                                    (blda->nw1[woff + l] + blda->p.b1) / \
                                    (blda->nkw1[l] + vb1);
                    if (l > 0){
                        prob[l] += prob[l - 1];
                    }
                }
            }
            else{
                blda->nw2[woff + top] -= 1;
                blda->nkw2[top] -= 1;
                for (int l = 0; l < blda->p.k; l++){
                    prob[l] = 1.0 * (blda->nd[doff  + l] + blda->p.a)  * \
                                    (blda->nw2[woff + l] + blda->p.b2) / \
                                    (blda->nkw2[l] + vb2);
                    if (l > 0){
                        prob[l] += prob[l - 1];
                    }
                }
            }

            double rnd = prob[blda->p.k - 1] * (0.1 + rand()) / (0.1 + RAND_MAX);
            for (st = 0; st < blda->p.k; st++){
                if (prob[st] > rnd) break;
            }

            blda->nd[doff + st] += 1;
            if (tpy == 1){
                blda->nw1[woff + st] += 1;
                blda->nkw1[st] += 1;
            }
            else{
                blda->nw2[woff + st] += 1;
                blda->nkw2[st] += 1;
            }
            blda->tokens[id][3] = st;
        }
        fprintf(stderr, " done\n");
        if (i % blda->p.savestep == 0){
            save_bri_lda(blda, i);
        }
    }
    free(p);    p = NULL;
    free(prob); prob = NULL;

}


void save_bri_lda(BriLda * blda, int n){
    FILE * fp = NULL;
    char nd_file[512];
    char nw1_file[512];
    char nw2_file[512];
    char tk_file[512];
    if (n < blda->p.niters){
        sprintf(nd_file,  "%s/%d_doc_topic",   blda->p.out_dir, n);
        sprintf(nw1_file, "%s/%d_word1_topic", blda->p.out_dir, n);
        sprintf(nw2_file, "%s/%d_word2_topic", blda->p.out_dir, n);
        sprintf(tk_file,  "%s/%d_token_topic", blda->p.out_dir, n);
    }
    else{
        sprintf(nd_file,  "%s/%s_doc_topic",   blda->p.out_dir, "f");
        sprintf(nw1_file, "%s/%s_word1_topic", blda->p.out_dir, "f");
        sprintf(nw2_file, "%s/%s_word2_topic", blda->p.out_dir, "f");
        sprintf(tk_file,  "%s/%s_token_topic", blda->p.out_dir, "f");
    }

    //output for nd
    if (NULL == (fp = fopen(nd_file, "w"))) {
        fprintf(stderr, "can not open file \"%s\"", nd_file);
        return;
    }
    for (int d = 0; d < blda->d; d++) {
        fprintf(fp, "%s", blda->id_doc_map[d]);
        for (int k = 0; k < blda->p.k; k++) {
            fprintf(fp, "\t%d", blda->nd[d * blda->p.k + k]);
        }
        fprintf(fp, "\n");
    }
    fclose(fp);

    //output for nw1
    if (NULL == (fp = fopen(nw1_file, "w"))) {
        fprintf(stderr, "can not open file \"%s\"", nw1_file);
        return;
    }
    for (int v1 = 0; v1 < blda->v1; v1++) {
        fprintf(fp, "%s", blda->id_v1_map[v1]);
        for (int k = 0; k < blda->p.k; k++) {
            fprintf(fp, "\t%d", blda->nw1[v1 * blda->p.k + k]);
        }
        fprintf(fp, "\n");
    }
    fclose(fp);

    //output for nw2
    if (NULL == (fp = fopen(nw2_file, "w"))) {
        fprintf(stderr, "can not open file \"%s\"", nw2_file);
        return;
    }
    for (int v2 = 0; v2 < blda->v2; v2++) {
        fprintf(fp, "%s", blda->id_v2_map[v2]);
        for (int k = 0; k < blda->p.k; k++) {
            fprintf(fp, "\t%d", blda->nw2[v2 * blda->p.k + k]);
        }             
        fprintf(fp, "\n");
    }
    fclose(fp);

    //output for tk
    if (NULL == (fp = fopen(tk_file, "w"))) {
        fprintf(stderr, "can not open file \"%s\"", tk_file);
        return;
    }
    for (int t = 0; t < blda->t; t++) {
        int type = blda->tokens[t][2];
        if (type == 1){
            fprintf(fp, "%s\t%s\t%d\t%d\n",  blda->id_doc_map[blda->tokens[t][0]],  \
                                             blda->id_v1_map[blda->tokens[t][1]],   \
                                             type, blda->tokens[t][3]);
        }
        else{
            fprintf(fp, "%s\t%s\t%d\t%d\n",  blda->id_doc_map[blda->tokens[t][0]],  \
                                             blda->id_v2_map[blda->tokens[t][1]],   \
                                             type, blda->tokens[t][3]);
        }
    }
    fclose(fp);
}

void free_bri_lda(BriLda * blda){
    if (blda){
        if (blda->id_doc_map){
            free(blda->id_doc_map);
            blda->id_doc_map = NULL;
        }
        if (blda->id_v1_map){
            free(blda->id_v1_map);
            blda->id_v1_map = NULL;
        }
        if (blda->id_v2_map){
            free(blda->id_v2_map);
            blda->id_v2_map = NULL;
        }
        if (blda->tokens){
            free(blda->tokens);
            blda->tokens = NULL;
        }
        if (blda->nd){
            free(blda->nd);
            blda->nd = NULL;
        }
        if (blda->nw1){
            free(blda->nw1);
            blda->nw1 = NULL;
        }
        if (blda->nw2){
            free(blda->nw2);
            blda->nw2 = NULL;
        }
        if (blda->nkw1){
            free(blda->nkw1);
            blda->nkw1 = NULL;
        }
        if (blda->nkw2){
            free(blda->nkw2);
            blda->nkw2 = NULL;
        }
        free(blda);
    }
}
