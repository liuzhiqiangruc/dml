/* ========================================================
 *   Copyright (C) 2015 All rights reserved.
 *   
 *   filename : joint_lda.c
 *   author   : ***
 *   date     : 2015-03-23
 *   info     : 
 * ======================================================== */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "idmap.h"
#include "str.h"
#include "joint_lda.h"

#define LDA_LINE_LEN 1024

static void malloc_space(JointLda * jlda){
    jlda->id_doc_map = (char(*)[KEY_SIZE])malloc(sizeof(char[KEY_SIZE]) * jlda->d);
    memset(jlda->id_doc_map, 0, sizeof(char[KEY_SIZE]) * jlda->d);
    
    jlda->id_v1_map  = (char(*)[KEY_SIZE])malloc(sizeof(char[KEY_SIZE]) * jlda->v1);
    memset(jlda->id_v1_map, 0, sizeof(char[KEY_SIZE]) * jlda->v1); 

    jlda->id_v2_map  = (char(*)[KEY_SIZE])malloc(sizeof(char[KEY_SIZE]) * jlda->v2);
    memset(jlda->id_v2_map, 0, sizeof(char[KEY_SIZE]) * jlda->v2); 

    jlda->tokens     = (int(*)[4])malloc(sizeof(int[4]) * jlda->t);
    memset(jlda->tokens, 0, sizeof(int[4]) * jlda->t); 

    jlda->nx         = (int*)malloc(sizeof(int) * jlda->d);
    memset(jlda->nx, 0, sizeof(int) * jlda->d);   

    jlda->kd         = (int*)malloc(sizeof(int) * jlda->p.k);
    memset(jlda->kd, 0, sizeof(int) * jlda->p.k);

    jlda->dk1        = (int*)malloc(sizeof(int) * jlda->d * jlda->p.k1);
    memset(jlda->dk1, 0, sizeof(int) * jlda->d * jlda->p.k1);

    jlda->dk2        = (int*)malloc(sizeof(int) * jlda->d * jlda->p.k2);
    memset(jlda->dk2, 0, sizeof(int) * jlda->d * jlda->p.k2);

    jlda->kk1        = (int*)malloc(sizeof(int) * jlda->p.k * jlda->p.k1);
    memset(jlda->kk1, 0, sizeof(int) * jlda->p.k * jlda->p.k1); 

    jlda->kk2        = (int*)malloc(sizeof(int) * jlda->p.k * jlda->p.k2);
    memset(jlda->kk2, 0, sizeof(int) * jlda->p.k * jlda->p.k2);  

    jlda->vk1        = (int*)malloc(sizeof(int) * jlda->v1 * jlda->p.k1);
    memset(jlda->vk1, 0, sizeof(int) * jlda->v1 * jlda->p.k1);

    jlda->vk2        = (int*)malloc(sizeof(int) * jlda->v2 * jlda->p.k2);
    memset(jlda->vk2, 0, sizeof(int) * jlda->v2 * jlda->p.k2);

    jlda->k1w        = (int*)malloc(sizeof(int) * jlda->p.k1);
    memset(jlda->k1w, 0, sizeof(int) * jlda->p.k1);

    jlda->k2w        = (int*)malloc(sizeof(int) * jlda->p.k2);
    memset(jlda->k2w, 0, sizeof(int) * jlda->p.k2);      
}


static void fullfill_param(JointLda * jlda){
    for (int d = 0; d < jlda->d; d++){
        int k = (int)((1.0 + rand())/(1.0 + RAND_MAX) * (jlda->p.k));
        jlda->nx[d] = k;
        jlda->kd[k] += 1;
    }
    for (int i = 0; i < jlda->t; i++){
        int uid = jlda->tokens[i][0];
        int vid = jlda->tokens[i][1];
        int tpy = jlda->tokens[i][2];
        int top = jlda->tokens[i][3];
        int k = jlda->nx[uid];
        if (tpy == 1){
            jlda->kk1[k   * jlda->p.k1 + top] += 1;
            jlda->vk1[vid * jlda->p.k1 + top] += 1;
            jlda->dk1[uid * jlda->p.k1 + top] += 1;
            jlda->k1w[top] += 1;
        }
        else if (tpy == 2){
            jlda->kk2[k   * jlda->p.k2 + top] += 1;
            jlda->vk2[vid * jlda->p.k2 + top] += 1;
            jlda->dk2[uid * jlda->p.k2 + top] += 1;
            jlda->k2w[top] += 1;
        }
    }
    save_joint_lda(jlda, 0);
}

static void shuffle(int * a, int n){
    int *tmp = a;
    int i,t,k;
    i = t = 0;
    for (k = n; k >= 2; k--){
        i = (int)((1.0 + rand()) / (RAND_MAX + 1.0) * k);
        t = tmp[i]; tmp[i] = tmp[0]; *tmp++ = t;
    }
}

static void update_nx(JointLda * jlda){
    double * p = (double*)malloc(sizeof(double) * jlda->p.k);
    for (int d = 0; d < jlda->d; d++){
        int dk = jlda->nx[d];
        int off_d1 = d * jlda->p.k1;
        int off_d2 = d * jlda->p.k2;

        jlda->kd[dk] -= 1;
        int off_dk = dk * jlda->p.k1;

        for (int k1 = 0; k1 < jlda->p.k1; k1++){
            jlda->kk1[off_dk + k1] -= jlda->dk1[off_d1 + k1];
        }

        off_dk = dk * jlda->p.k2;
        for (int k2 = 0; k2 < jlda->p.k2; k2++){
            jlda->kk2[off_dk + k2] -= jlda->dk2[off_d2 + k2];
        }

        double mxp = -1.79e308;
        memset(p, 0, sizeof(double) * jlda->p.k);
        for (int k = 0; k < jlda->p.k; k++){
            int ds = 0, ks = 0;
            int off_kk = k * jlda->p.k1;
            for (int k1 = 0; k1 < jlda->p.k1; k1++){
                int kk1 = jlda->kk1[off_kk + k1];
                int dk1 = jlda->dk1[off_d1 + k1];
                ks += kk1; ds += dk1;
                p[k] += dk1 * log(kk1 + jlda->p.a1);
            }
            p[k] -= ds * log(ks + jlda->p.a1 * jlda->p.k1);

            ds = ks = 0;
            off_kk = k * jlda->p.k2;
            for (int k2 = 0; k2 < jlda->p.k2; k2++){
                int kk2 = jlda->kk2[off_kk + k2];
                int dk2 = jlda->dk1[off_d2 + k2];
                ks += kk2; ds += dk2;
                p[k] += dk2 * log(kk2 + jlda->p.a2);
            }
            p[k] -= ds * log(ks + jlda->p.a2 * jlda->p.k2);
            p[k] += log(jlda->kd[k] + jlda->p.a);
            if (mxp < p[k]) mxp = p[k];
        }
        for (int k = 0; k < jlda->p.k; k++){
            p[k] = exp(p[k] - mxp);
            if (k > 0) p[k] += p[k - 1];
        }
        double rnd = p[jlda->p.k - 1] * (0.1 + rand()) / (0.1 + RAND_MAX);
        int newk = 0;
        for (; newk < jlda->p.k; newk++){
            if (p[newk] > rnd) break;
        }
        if (newk == jlda->p.k){
            fprintf(stderr,"sample x failed\n");
            newk = (int)jlda->p.k * (0.1 + rand()) / (0.1 + RAND_MAX);
        }
        jlda->nx[d] = newk;
        jlda->kd[newk] += 1;

        off_dk = newk * jlda->p.k1;
        for (int k1 = 0; k1 < jlda->p.k1; k1++){
            jlda->kk1[off_dk + k1] += jlda->dk1[off_d1 + k1];
        }
        off_dk = newk * jlda->p.k2;
        for (int k2 = 0; k2 < jlda->p.k2; k2++){
            jlda->kk2[off_dk + k2] += jlda->dk2[off_d2 + k2];
        }
    }
    free(p); p = NULL;
}

JointLda * create_joint_lda(){
    JointLda * jlda = (JointLda*)malloc(sizeof(JointLda));
    memset(jlda, 0, sizeof(JointLda));
    return jlda;
}


int init_joint_lda(JointLda * jlda){
    FILE * fp = NULL;
    if (NULL == (fp = fopen(jlda->p.in_file,"r"))){
        fprintf(stderr,"can not open file \"%s\"\n", jlda->p.in_file);
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

    jlda->d  = idmap_size(uidmap);
    jlda->t  = token_size;
    jlda->v1 = idmap_size(v1idmap);
    jlda->v2 = idmap_size(v2idmap);

    malloc_space(jlda);

    rewind(fp);
    int uid = -1, v1id = -1, v2id = -1, tid = -1;
    int token_index = 0;

    while (NULL != fgets(buffer, LDA_LINE_LEN, fp)){
        str_array = split(trim(buffer, 3), '\t', &count);
        if (count < 3){
            goto str_free;
        }
        type = atoi(str_array[2]);
        if (type == 1){
            v1id = idmap_get_value(v1idmap, str_array[1]);
            strncpy(jlda->id_v1_map[v1id], str_array[1], KEY_SIZE - 1);
            jlda->tokens[token_index][1] = v1id;
        }
        else if (type == 2){
            v2id = idmap_get_value(v2idmap, str_array[1]);
            strncpy(jlda->id_v2_map[v2id], str_array[1], KEY_SIZE - 1);
            jlda->tokens[token_index][1] = v2id;
        }
        else{
            goto str_free;
        }
        jlda->tokens[token_index][2] = type;

        uid = idmap_get_value(uidmap,str_array[0]);
        strncpy(jlda->id_doc_map[uid], str_array[0], KEY_SIZE - 1);
        jlda->tokens[token_index][0] = uid;


        if (count == 4){
            tid = atoi(str_array[3]);
        }
        else if (type == 1){
            tid = (int)((1.0 + rand()) / (1.0 + RAND_MAX) * (jlda->p.k1));
        }
        else if (type == 2){
            tid = (int)((1.0 + rand()) / (1.0 + RAND_MAX) * (jlda->p.k2));
        }
        jlda->tokens[token_index][3] = tid;

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



void est_joint_lda(JointLda * jlda){

    fullfill_param(jlda);

    int * p = (int*)malloc(sizeof(int) * jlda->t);
    int st = 0;

    double * prob1 = (double*)malloc(sizeof(double) * jlda->p.k1);
    double * prob2 = (double*)malloc(sizeof(double) * jlda->p.k2);
    double vb1 = jlda->p.b1 * jlda->v1;
    double vb2 = jlda->p.b2 * jlda->v2;

    for (int i = 0; i< jlda->t; i++) p[i] = i;
    for (int i = 1; i <= jlda->p.niters; i++){
        fprintf(stderr,"iteration %d estimate begin ... ", i);
        shuffle(p,jlda->t);
        //update z
        for (int j = 0; j < jlda->t; j++){
            int id = p[j];
            int uid = jlda->tokens[id][0];
            int vid = jlda->tokens[id][1];
            int tpy = jlda->tokens[id][2];
            int top = jlda->tokens[id][3];
            int uk  = jlda->nx[uid];
            if (tpy == 1){
                jlda->dk1[uid * jlda->p.k1 + top] -= 1;
                jlda->kk1[uk  * jlda->p.k1 + top] -= 1;
                jlda->vk1[vid * jlda->p.k1 + top] -= 1;
                jlda->k1w[top] -= 1;
                for (int l = 0; l < jlda->p.k1; l++){
                    prob1[l] = 1.0 * (jlda->kk1[uk  * jlda->p.k1 + l] + jlda->p.a1) * \
                                     (jlda->vk1[vid * jlda->p.k1 + l] + jlda->p.b1) / \
                                     (jlda->k1w[l] + vb1);
                    if (l > 0) prob1[l] += prob1[l - 1];
                }
                double rnd = prob1[jlda->p.k1 - 1] * (0.1 + rand()) / (0.1 + RAND_MAX);
                for (st = 0; st < jlda->p.k1; st++){
                    if (prob1[st] > rnd) break;
                }
                jlda->dk1[uid * jlda->p.k1 + st] += 1;
                jlda->kk1[uk  * jlda->p.k1 + st] += 1;
                jlda->vk1[vid * jlda->p.k1 + st] += 1;
                jlda->k1w[st] += 1;
            }
            else if (tpy == 2){
                jlda->dk2[uid * jlda->p.k2 + top] -= 1;
                jlda->kk2[uk  * jlda->p.k2 + top] -= 1;
                jlda->vk2[vid * jlda->p.k2 + top] -= 1;
                jlda->k2w[top] -= 1;
                for (int l = 0; l < jlda->p.k2; l++){
                    prob2[l] = 1.0 * (jlda->kk2[uk  * jlda->p.k2 + l] + jlda->p.a2) * \
                                     (jlda->vk2[vid * jlda->p.k2 + l] + jlda->p.b2) / \
                                     (jlda->k2w[l] + vb2);
                    if (l > 0) prob2[l] += prob2[l - 1];
                }
                double rnd = prob2[jlda->p.k2 - 1] * (0.1 + rand()) / (0.1 + RAND_MAX);
                for (st = 0; st < jlda->p.k2; st++){
                    if (prob2[st] > rnd) break;
                }
                jlda->dk2[uid * jlda->p.k2 + st] += 1;
                jlda->kk2[uk  * jlda->p.k2 + st] += 1;
                jlda->vk2[vid * jlda->p.k2 + st] += 1;
                jlda->k2w[st] += 1;
            }
            jlda->tokens[id][3] = st;
        }
        fprintf(stderr," done\n");
        if (i % jlda->p.savestep == 0){
            update_nx(jlda);
            save_joint_lda(jlda, i);
        }
    }
    free(p); p = NULL;
    free(prob1); prob1 = NULL;
    free(prob2); prob2 = NULL;
}

void save_joint_lda(JointLda * jlda, int n){
    FILE * fp = NULL;
    char nx_file[512];
    char dk1_file[512];
    char dk2_file[512];
    char vk1_file[512];
    char vk2_file[512];
    char kk1_file[512];
    char kk2_file[512];
    char tkn_file[512];
    if (n < jlda->p.niters){
        sprintf(nx_file,  "%s/%d_nx_file",  jlda->p.out_dir, n);
        sprintf(dk1_file, "%s/%d_dk1_file", jlda->p.out_dir, n);
        sprintf(dk2_file, "%s/%d_dk2_file", jlda->p.out_dir, n);
        sprintf(vk1_file, "%s/%d_vk1_file", jlda->p.out_dir, n);
        sprintf(vk2_file, "%s/%d_vk2_file", jlda->p.out_dir, n);
        sprintf(kk1_file, "%s/%d_kk1_file", jlda->p.out_dir, n);
        sprintf(kk2_file, "%s/%d_kk2_file", jlda->p.out_dir, n);
        sprintf(tkn_file, "%s/%d_tkn_file", jlda->p.out_dir, n);
    }else{
        sprintf(nx_file,  "%s/%s_nx_file",  jlda->p.out_dir, "f");
        sprintf(dk1_file, "%s/%s_dk1_file", jlda->p.out_dir, "f");
        sprintf(dk2_file, "%s/%s_dk2_file", jlda->p.out_dir, "f");
        sprintf(vk1_file, "%s/%s_vk1_file", jlda->p.out_dir, "f");
        sprintf(vk2_file, "%s/%s_vk2_file", jlda->p.out_dir, "f");
        sprintf(kk1_file, "%s/%s_kk1_file", jlda->p.out_dir, "f");
        sprintf(kk2_file, "%s/%s_kk2_file", jlda->p.out_dir, "f");
        sprintf(tkn_file, "%s/%s_tkn_file", jlda->p.out_dir, "f");
    }

    // output nx
    if (NULL == (fp = fopen(nx_file,"w"))){
        fprintf(stderr,"can not open file \"%s\"", nx_file);
        return;
    }
    for (int d = 0; d < jlda->d; d++){
        fprintf(fp, "%d\n", jlda->nx[d]);
    }
    fclose(fp);

    // output dk1
    if (NULL == (fp = fopen(dk1_file,"w"))){
        fprintf(stderr,"can not open file \"%s\"", dk1_file);
        return;
    }
    for (int d = 0; d < jlda->d; d++){
        fprintf(fp, "%s", jlda->id_doc_map[d]);
        for (int k1 = 0; k1 < jlda->p.k1; k1++){
            fprintf(fp, "\t%d", jlda->dk1[d * jlda->p.k1 + k1]);
        }
        fprintf(fp, "\n");
    }
    fclose(fp);

    // output dk2
    if (NULL == (fp = fopen(dk2_file,"w"))){
        fprintf(stderr,"can not open file \"%s\"", dk2_file);
        return;
    }
    for (int d = 0; d < jlda->d; d++){
        fprintf(fp, "%s", jlda->id_doc_map[d]);
        for (int k2 = 0; k2 < jlda->p.k2; k2++){
            fprintf(fp, "\t%d", jlda->dk2[d * jlda->p.k2 + k2]);
        }
        fprintf(fp, "\n");
    }
    fclose(fp);

    // output vk1
    if (NULL == (fp = fopen(vk1_file,"w"))){
        fprintf(stderr,"can not open file \"%s\"", vk1_file);
        return;
    }
    for (int v1 = 0; v1 < jlda->v1; v1++){
        fprintf(fp, "%s", jlda->id_v1_map[v1]);
        for (int k1 = 0; k1 < jlda->p.k1; k1++){
            fprintf(fp, "\t%d", jlda->vk1[v1 * jlda->p.k1 + k1]);
        }
        fprintf(fp, "\n");
    }
    fclose(fp);

    // output vk2
    if (NULL == (fp = fopen(vk2_file,"w"))){
        fprintf(stderr,"can not open file \"%s\"", vk2_file);
        return;
    }
    for (int v2 = 0; v2 < jlda->v2; v2++){
        fprintf(fp, "%s", jlda->id_v2_map[v2]);
        for (int k2 = 0; k2 < jlda->p.k2; k2++){
            fprintf(fp, "\t%d", jlda->vk2[v2 * jlda->p.k2 + k2]);
        }
        fprintf(fp, "\n");
    }
    fclose(fp);

    // output kk1
    if (NULL == (fp = fopen(kk1_file, "w"))){
        fprintf(stderr,"can not open file \"%s\"\n", kk1_file);
        return;
    }
    for (int k = 0; k < jlda->p.k; k++){
        for (int k1 = 0; k1 < jlda->p.k1; k1++){
            fprintf(fp, "%d\t", jlda->kk1[k * jlda->p.k1 + k1]);
        }
        fprintf(fp, "\n");
    }
    fclose(fp);

    // output kk2
    if (NULL == (fp = fopen(kk2_file, "w"))){
        fprintf(stderr,"can not open file \"%s\"\n", kk2_file);
        return;
    }
    for (int k = 0; k < jlda->p.k; k++){
        for (int k2 = 0; k2 < jlda->p.k2; k2++){
            fprintf(fp, "%d\t", jlda->kk2[k * jlda->p.k2 + k2]);
        }
        fprintf(fp, "\n");
    }
    fclose(fp);

    // output tkn 
    if (NULL == (fp = fopen(tkn_file, "w"))){
        fprintf(stderr,"can not open file \"%s\"\n", tkn_file);
        return;
    }
    for (int t = 0; t < jlda->t; t++){
        int type = jlda->tokens[t][2];
        if (type == 1){
            fprintf(fp, "%s\t%s\t%d\t%d\n", jlda->id_doc_map[jlda->tokens[t][0]], \
                                            jlda->id_v1_map[jlda->tokens[t][1]],  \
                                            type, jlda->tokens[t][3]);
        }
        else {
            fprintf(fp, "%s\t%s\t%d\t%d\n", jlda->id_doc_map[jlda->tokens[t][0]], \
                                            jlda->id_v2_map[jlda->tokens[t][1]],  \
                                            type, jlda->tokens[t][3]);
        }
    }
    fclose(fp);
}

void free_joint_lda(JointLda * jlda){
    if (jlda){
        if (jlda->id_doc_map){
            free(jlda->id_doc_map);
            jlda->id_doc_map = NULL;
        }
        if (jlda->id_v1_map){
            free(jlda->id_v1_map);
            jlda->id_v1_map = NULL;
        }
        if (jlda->id_v2_map){
            free(jlda->id_v2_map);
            jlda->id_v2_map = NULL;
        }
        if (jlda->tokens){
            free(jlda->tokens);
            jlda->tokens = NULL;
        }
        if (jlda->nx){
            free(jlda->nx);
            jlda->nx = NULL;
        }
        if (jlda->kd){
            free(jlda->kd);
            jlda->kd = NULL;
        }
        if (jlda->dk1){
            free(jlda->dk1);
            jlda->dk1 = NULL;
        }
        if (jlda->dk2){
            free(jlda->dk2);
            jlda->dk2 = NULL;
        }
        if (jlda->kk1){
            free(jlda->kk1);
            jlda->kk1 = NULL;
        }
        if (jlda->kk2){
            free(jlda->kk2);
            jlda->kk2 = NULL;
        }
        if (jlda->vk1){
            free(jlda->vk1);
            jlda->vk1 = NULL;
        }
        if (jlda->vk2){
            free(jlda->vk2);
            jlda->vk2 = NULL;
        }
        if (jlda->k1w){
            free(jlda->k1w);
            jlda->k1w = NULL;
        }
        if (jlda->k2w){
            free(jlda->k2w);
            jlda->k2w = NULL;
        }
        free(jlda);
    }
}


