/* ========================================================
 *   Copyright (C) 2015 All rights reserved.
 *   
 *   filename : author_lda.c
 *   author   : liuzhiqiang01@baidu.com
 *   date     : 2015-04-02
 *   info     : implementation of author topic model
 * ======================================================== */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "str.h"
#include "idmap.h"
#include "author_lda.h"

#define  LDA_LINE_LEN 1024
#define  LDA_AUTH_LEN 16384


static int load_author(AuthorLda * alda, IdMap * uidmap, IdMap * aidmap){
    FILE * fp = NULL;
    if (NULL == (fp = fopen(alda->p.author_file,"r"))){
        fprintf(stderr,  "can not open author file \"%s\"", alda->p.author_file);
        return -1;
    }

    char buffer[LDA_AUTH_LEN];
    char ** str_array = NULL;
    int count = 0, c = 0;
    while (NULL !=fgets(buffer, LDA_AUTH_LEN, fp)){
        str_array = split(trim(buffer, 3), '\t', &count);
        if (count < 4){
            goto free_str;
        }
        c = atoi(str_array[1]);
        if ((c + 1) << 1 != count){
            goto free_str;
        }

        if (alda->MA < c){
            alda->MA = c;
        }

        if (-1 == idmap_get_value(uidmap, str_array[0])){
            idmap_add(uidmap, dupstr(str_array[0]), idmap_size(uidmap));
        }

        for (int i = 0; i < c; i++){
            int j = (i + 1) << 1;
            if (-1 == idmap_get_value(aidmap, str_array[j])) {
                idmap_add(aidmap, dupstr(str_array[j]), idmap_size(aidmap));
            }
        }

free_str:
        free(str_array[0]);
        free(str_array);
    }

    alda->D = idmap_size(uidmap);
    alda->A = idmap_size(aidmap);

    alda->doc_author = (DocAuthors*)malloc(sizeof(DocAuthors) * alda->D);
    memset(alda->doc_author, 0, sizeof(DocAuthors) * alda->D);

    alda->id_doc_map = (char(*)[KEY_SIZE])malloc(sizeof(char[KEY_SIZE]) * alda->D);
    memset(alda->id_doc_map, 0, sizeof(char[KEY_SIZE]) * alda->D);

    alda->id_au_map = (char(*)[KEY_SIZE])malloc(sizeof(char[KEY_SIZE]) * alda->A);
    memset(alda->id_au_map, 0, sizeof(char[KEY_SIZE]) * alda->A);

    alda->nka = (int*)malloc(sizeof(int) * alda->A);
    memset(alda->nka, 0, sizeof(int) * alda->A);

    alda->na = (int*)malloc(sizeof(int) * alda->A * alda->p.k);
    memset(alda->na, 0, sizeof(int) * alda->A * alda->p.k);

    rewind(fp);

    int uid = -1, aindex = 0, aid = 0, ant = 0;
    while (NULL != fgets(buffer, LDA_AUTH_LEN, fp)) {
        str_array = split(trim(buffer, 3), '\t', &count);

        if (count < 4){
            goto str_free;
        }

        c = atoi(str_array[1]);
        if ((c + 1) << 1 != count){
            goto str_free;
        }

        uid = idmap_get_value(uidmap, str_array[0]);
        strncpy(alda->id_doc_map[uid],str_array[0], KEY_SIZE - 1);

        DocAuthors * ad = alda->doc_author + uid;
        ad->nda = c;
        ad->das = (int(*)[2])malloc(sizeof(int[2]) * c);
        memset(ad->das, 0, sizeof(int[2]) * c);

        for (int i = 0; i < c; i++) {
            aindex = (i + 1) << 1;
            aid    = idmap_get_value(aidmap, str_array[aindex]);
            ant   = atoi(str_array[aindex + 1]);
            ad->das[i][0] = aid;
            ad->das[i][1] = ant;
            ad->csum     += ant;
            strncpy(alda->id_au_map[aid], str_array[aindex], KEY_SIZE - 1);
        }

str_free:
        free(str_array[0]);
        free(str_array);
    }

    fclose(fp);

    return 0;
}

static int load_tokens(AuthorLda * alda, IdMap * uidmap, IdMap * vidmap, IdMap * aidmap){
    FILE * fp = NULL;
    if ( NULL == (fp = fopen(alda->p.in_file,"r"))){
        fprintf(stderr, "can not open file \"%s\"\n", alda->p.in_file);
        return -1;
    }

    char buffer[LDA_LINE_LEN];
    char ** str_array = NULL;
    int count = 0, token_size = 0, uid = -1, vid = -1, tid = -1, aid = -1, aiid = -1;

    while (NULL != fgets(buffer, LDA_LINE_LEN, fp)){

        str_array = split(trim(buffer,3), '\t', &count);
        if (count != 2 && count != 4){
            goto free_str;
        }

        uid = idmap_get_value(uidmap, str_array[0]);
        if (-1 == uid){
            goto free_str;
        }

        if (-1 == idmap_get_value(vidmap, str_array[1])) {
            idmap_add(vidmap, dupstr(str_array[1]), idmap_size(vidmap));
        }

        token_size += 1;

free_str:
        free(str_array[0]);
        free(str_array);
    }

    alda->V = idmap_size(vidmap);
    alda->T = token_size;

    alda->id_v_map = (char(*)[KEY_SIZE])malloc(sizeof(char[KEY_SIZE]) * alda->V);
    memset(alda->id_v_map, 0, sizeof(char[KEY_SIZE]) * alda->V);

    alda->tokens = (int(*)[4])malloc(sizeof(int[4]) * alda->T);
    memset(alda->tokens, 0, sizeof(int[4]) * alda->T);

    alda->nw = (int*)malloc(sizeof(int) * alda->V * alda->p.k);
    memset(alda->nw, 0, sizeof(int) * alda->V * alda->p.k);

    rewind(fp);
    token_size = 0;

    while (NULL != fgets(buffer, LDA_LINE_LEN, fp)) {
        str_array = split(trim(buffer,3), '\t', &count);

        if (count != 2 && count != 4){
            goto str_free;
        }

        uid = idmap_get_value(uidmap, str_array[0]);
        if (-1 == uid){
            goto str_free;
        }

        vid = idmap_get_value(vidmap, str_array[1]);
        strncpy(alda->id_v_map[vid], str_array[1], KEY_SIZE - 1);

        alda->tokens[token_size][0] = uid;
        alda->tokens[token_size][1] = vid;

        DocAuthors * ad = alda->doc_author + uid;
        aiid = (int)(ad->nda * (0.1 + rand()) / (0.1 + RAND_MAX));
        aid = ad->das[aiid][0];
        if (count == 4){
            aid = idmap_get_value(aidmap, str_array[2]);
            if (-1 == aid){
                fprintf(stderr,"this can not happen\n");
                exit(-1);
            }
        }
        alda->tokens[token_size][2] = aid;

        tid = (int)(alda->p.k * (0.1 + rand()) / (0.1 + RAND_MAX));
        if (count == 4){
            tid = atoi(str_array[3]);
        }
        alda->tokens[token_size][3] = tid;

        token_size += 1;
        
str_free:
        free(str_array[0]);
        free(str_array);
    }
    fclose(fp);

    return 0;
}

static int load_phi(AuthorLda * alda, IdMap * vidmap){
    FILE * fp = NULL;
    if (NULL == (fp = fopen(alda->p.phi_file, "r"))){
        fprintf(stderr, "phi file is not valid\n");
        return -1;
    }
    char buffer[LDA_LINE_LEN];
    char ** str_array = NULL;
    int count = 0, vid = 0, voff = 0, vcnt = 0;
    while(NULL != fgets(buffer, LDA_LINE_LEN, fp)){
        str_array = split(trim(buffer,3), '\t', &count);
        if (count != alda->p.k + 1){
            goto free_str;
        }
        vid = idmap_get_value(vidmap, str_array[0]);
        if (vid == -1){
            goto free_str;
        }
        vcnt += 1;
        voff = vid * alda->p.k;
        for (int i = 0; i < alda->p.k; i++){
            alda->nw[voff + i] = atoi(str_array[i + 1]);
            alda->nkw[i] += atoi(str_array[i + 1]);
        }
free_str:
        free(str_array[0]);
        free(str_array);
    }
    fprintf(stderr, "load phi done, vcnt = %d\n", vcnt);
    return 0;
}


static void fullfill_param(AuthorLda * alda){
    for (int t = 0; t < alda->T; t++){
        int vid = alda->tokens[t][1];
        int aid = alda->tokens[t][2];
        int tid = alda->tokens[t][3];
        alda->na[aid * alda->p.k + tid] += 1;
        alda->nka[aid] += 1;
#ifndef PHI
        alda->nw[vid * alda->p.k + tid] += 1;
        alda->nkw[tid] += 1;
#endif
    }
    save_author_lda(alda, 0);
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

AuthorLda * create_author_lda(){
    AuthorLda * alda = (AuthorLda*)malloc(sizeof(AuthorLda));
    memset(alda, 0, sizeof(AuthorLda));
    return alda;
}

int init_author_lda(AuthorLda * alda){

    IdMap * uidmap = idmap_create();
    IdMap * vidmap = idmap_create();
    IdMap * aidmap = idmap_create();

    if (0 != load_author(alda, uidmap, aidmap)){
        goto free_maps;
    }
    fprintf(stderr, "author load done, doc : %d, author : %d\n", alda->D, alda->A);

    if (0 != load_tokens(alda, uidmap, vidmap, aidmap)){
        goto free_author;
    }
    fprintf(stderr, "tokens laod done, tokens : %d, words : %d\n", alda->T, alda->V);

    alda->nkw = (int*)malloc(sizeof(int) * alda->p.k);
    memset(alda->nkw, 0, sizeof(int) * alda->p.k);

#ifdef PHI
    if (0 != load_phi(alda, vidmap)){
        goto free_author;
    }
    fprintf(stderr, "phi load done\n");
#endif

    return 0;

free_author:
    free_author_lda(alda);

free_maps:
    idmap_free(uidmap);   uidmap = NULL;
    idmap_free(vidmap);   vidmap = NULL;
    idmap_free(aidmap);   aidmap = NULL;

    return -1;
}

void est_author_lda(AuthorLda * alda){

    fullfill_param(alda);

    int *p = (int*)malloc(sizeof(int) * alda->T);
    int st = 0, sa = 0;

    double vb = alda->V   * alda->p.b;
    double at = alda->p.a * alda->p.k;
    double * prob = (double*)malloc(sizeof(double) * alda->MA * alda->p.k);

    for (int i = 0; i < alda->T; i++) {
        p[i] = i;
    }

    for (int i = 1; i <= alda->p.niters; i++){
        fprintf(stderr, "iteration %d estimate begin ... ", i);
        shuffle(p, alda->T);
        for (int j = 0; j < alda->T; j++){
            int id = p[j];
            int uid = alda->tokens[id][0];
            int vid = alda->tokens[id][1];
            int aid = alda->tokens[id][2];
            int tid = alda->tokens[id][3];
            int v_off = vid * alda->p.k;

            alda->na[aid * alda->p.k + tid] -= 1;
            alda->nka[aid]                  -= 1;
#ifndef PHI
            alda->nw[vid * alda->p.k + tid] -= 1;
            alda->nkw[tid]                  -= 1;
#endif

            DocAuthors * ad = alda->doc_author + uid;

            memset(prob, 0, sizeof(double) * alda->MA * alda->p.k);
            for (int ai = 0; ai < ad->nda; ai++) {

                int a      = ad->das[ai][0];
                int ai_off = ai * alda->p.k;
                int a_off  = a  * alda->p.k;

                for (int l = 0; l < alda->p.k; l++){
                    prob[ai_off + l] = 1.0 * (alda->na[a_off + l] + alda->p.a) \
                                           * (alda->nw[v_off + l] + alda->p.b) \
                                           / (alda->nka[a]        + at)        \
                                           / (alda->nkw[l]        + vb)        \
                                           * (ad->das[ai][1]);
                    if (ai_off + l > 0){
                        prob[ai_off + l] += prob[ai_off + l - 1];
                    }
                }
            }

            double rnd = 1.0 * prob[ad->nda * alda->p.k - 1] * rand() / (1.0 + RAND_MAX);
            int sampled_index = 0;
            for (; sampled_index < ad->nda * alda->p.k; sampled_index++){
                if (prob[sampled_index] > rnd) break;
            }
            if (sampled_index == ad->nda * alda->p.k){
                // just keep as before
                fprintf(stderr, "\n[warning] sampled author and topic failed\n");
                // and see what is going on
                fprintf(stderr, "token index :%d \n", id);
                for (int ti = 0; ti < ad->nda; ti++){
                    int tioff = ti * alda->p.k;
                    for (int tk = 0; tk < alda->p.k; tk++){
                        fprintf(stderr, "% e", prob[tioff + tk]);
                    }
                    fprintf(stderr, "\n");
                }
                fprintf(stderr,"rnd: %e\n", rnd);
                save_author_lda(alda,i);
                exit(2);
                sa = aid;
                st = tid;
            }else {
                sa = ad->das[sampled_index / alda->p.k][0];
                st = sampled_index % alda->p.k;
            }

            alda->na[sa  * alda->p.k + st] += 1;
            alda->nka[sa]                  += 1;
#ifndef PHI
            alda->nw[vid * alda->p.k + st] += 1;
            alda->nkw[st]                  += 1;
#endif

            alda->tokens[id][2] = sa;
            alda->tokens[id][3] = st;
        }
        fprintf(stderr, " done\n");
        if (i % alda->p.savestep == 0){
            save_author_lda(alda, i);
        }
    }
    free(p);    p    = NULL;
    free(prob); prob = NULL;

}
void   save_author_lda(AuthorLda * alda, int n){
    FILE * fp = NULL;
    char na_file[512];
    char nw_file[512];
    char tk_file[512];

    if (n < alda->p.niters){
        sprintf(na_file,  "%s/%d_auth_topic", alda->p.out_dir, n);
        sprintf(nw_file,  "%s/%d_word_topic", alda->p.out_dir, n);
        sprintf(tk_file,  "%s/%d_token_topic",alda->p.out_dir, n);
    }
    else{
        sprintf(na_file,  "%s/%s_auth_topic", alda->p.out_dir, "f");
        sprintf(nw_file,  "%s/%s_word_topic", alda->p.out_dir, "f");
        sprintf(tk_file,  "%s/%s_token_topic",alda->p.out_dir, "f");
    }

    //output for nd
    if (NULL == (fp = fopen(na_file, "w"))) {
        fprintf(stderr, "can not open file \"%s\"", na_file);
        return;
    }
    for (int a = 0; a < alda->A; a++) {
        fprintf(fp, "%s", alda->id_au_map[a]);
        for (int k = 0; k < alda->p.k; k++) {
            fprintf(fp, "\t%d", alda->na[a * alda->p.k + k]);
        }
        fprintf(fp, "\n");
    }
    fclose(fp);

    //output for nw
    if (NULL == (fp = fopen(nw_file, "w"))) {
        fprintf(stderr, "can not open file \"%s\"", nw_file);
        return;
    }
    for (int v = 0; v < alda->V; v++) {
        fprintf(fp, "%s", alda->id_v_map[v]);
        for (int k = 0; k < alda->p.k; k++) {
            fprintf(fp, "\t%d", alda->nw[v * alda->p.k + k]);
        }
        fprintf(fp, "\n");
    }
    fclose(fp);

    //output for tk
    if (NULL == (fp = fopen(tk_file, "w"))) {
        fprintf(stderr, "can not open file \"%s\"", tk_file);
        return;
    }
    for (int t = 0; t < alda->T; t++) {
        fprintf(fp, "%s\t%s\t%s\t%d\n",  alda->id_doc_map[alda->tokens[t][0]],  \
                                         alda->id_v_map[alda->tokens[t][1]],    \
                                         alda->id_au_map[alda->tokens[t][2]],   \
                                         alda->tokens[t][3]);
    }
    fclose(fp);
}

void free_author_lda(AuthorLda * alda){
    if (alda){
        if (alda->doc_author){
            for (int d = 0; d < alda->D; d++){
                if (alda->doc_author[d].das){
                    free(alda->doc_author[d].das);
                    alda->doc_author[d].das = NULL;
                }
            }
            free(alda->doc_author);
            alda->doc_author = NULL;
        }
        if (alda->id_doc_map){
            free(alda->id_doc_map);
            alda->id_doc_map = NULL;
        }
        if (alda->id_v_map){
            free(alda->id_v_map);
            alda->id_v_map = NULL;
        }
        if (alda->id_au_map){
            free(alda->id_au_map);
            alda->id_au_map = NULL;
        }
        if (alda->tokens){
            free(alda->tokens);
            alda->tokens = NULL;
        }
        if (alda->na){
            free(alda->na);
            alda->na = NULL;
        }
        if (alda->nw){
            free(alda->nw);
            alda->nw = NULL;
        }
        if (alda->nka){
            free(alda->nka);
            alda->nka = NULL;
        }
        if (alda->nkw){
            free(alda->nkw);
            alda->nkw = NULL;
        }
        free(alda);
    }
}

