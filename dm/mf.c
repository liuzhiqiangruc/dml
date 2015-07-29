/* ========================================================
 *   Copyright (C) 2015 All rights reserved.
 *   
 *   filename : mf.c
 *   author   : ***
 *   date     : 2015-05-25
 *   info     : 
 * ======================================================== */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "idmap.h"
#include "str.h"
#include "mf.h"

#define MF_LINE_LEN 1024


static void malloc_train(MF *mf) {
    mf->id_u_map = (char(*)[KEY_SIZE])malloc(sizeof(char[KEY_SIZE]) * mf->U);
    mf->id_i_map = (char(*)[KEY_SIZE])malloc(sizeof(char[KEY_SIZE]) * mf->I);

    memset(mf->id_u_map, 0, sizeof(char[KEY_SIZE]) * mf->U); 
    memset(mf->id_i_map, 0, sizeof(char[KEY_SIZE]) * mf->I); 

    mf->u_i = (int(*)[2])malloc(sizeof(int[2]) * mf->T);
    mf->s   =   (double*)malloc(sizeof(double) * mf->T);

    memset(mf->u_i, 0, sizeof(int[2]) * mf->T);
    memset(mf->s,   0, sizeof(double) * mf->T);

    mf->pu     = (double*)malloc(sizeof(double) * mf->U * mf->p.k);
    mf->pu_tmp = (double*)malloc(sizeof(double) * mf->U * mf->p.k);

    memset(mf->pu,     0, sizeof(double) * mf->U * mf->p.k);  
    memset(mf->pu_tmp, 0, sizeof(double) * mf->U * mf->p.k);  

    mf->qi     = (double*)malloc(sizeof(double) * mf->I * mf->p.k);
    mf->qi_tmp = (double*)malloc(sizeof(double) * mf->I * mf->p.k);

    memset(mf->qi,     0, sizeof(double) * mf->I * mf->p.k);   
    memset(mf->qi_tmp, 0, sizeof(double) * mf->I * mf->p.k);   

    mf->bu     = (double*)malloc(sizeof(double) * mf->U);
    mf->bu_tmp = (double*)malloc(sizeof(double) * mf->U);

    memset(mf->bu,     0, sizeof(double) * mf->U);       
    memset(mf->bu_tmp, 0, sizeof(double) * mf->U);       

    mf->bi     = (double*)malloc(sizeof(double) * mf->I);
    mf->bi_tmp = (double*)malloc(sizeof(double) * mf->I);

    memset(mf->bi,     0, sizeof(double) * mf->I);
    memset(mf->bi_tmp, 0, sizeof(double) * mf->I);
}

static void malloc_test(MF *mf) {
    mf->u_t = (int(*)[2])malloc(sizeof(int[2]) * mf->TT);
    mf->s_t =   (double*)malloc(sizeof(double) * mf->TT);
    memset(mf->u_t, 0, sizeof(int[2]) * mf->TT);
    memset(mf->s_t, 0, sizeof(double) * mf->TT);
}

void backup(MF * mf){
    memmove(mf->pu_tmp, mf->pu, sizeof(double) * mf->U * mf->p.k);
    memmove(mf->qi_tmp, mf->qi, sizeof(double) * mf->I * mf->p.k);
    memmove(mf->bu_tmp, mf->bu, sizeof(double) * mf->U);
    memmove(mf->bi_tmp, mf->bi, sizeof(double) * mf->I);
}

void recover(MF * mf){
    memmove(mf->pu, mf->pu_tmp, sizeof(double) * mf->U * mf->p.k);
    memmove(mf->qi, mf->qi_tmp, sizeof(double) * mf->I * mf->p.k);
    memmove(mf->bu, mf->bu_tmp, sizeof(double) * mf->U);
    memmove(mf->bi, mf->bi_tmp, sizeof(double) * mf->I);
}

MF * create_mf(){
    MF * mf = (MF*)malloc(sizeof(MF));
    memset(mf, 0, sizeof(MF));
    return mf;
}

int  init_mf(MF * mf){
    FILE * fp = NULL;
    FILE * fp_t = NULL;
    if (NULL == (fp = fopen(mf->p.in_file, "r"))){
        fprintf(stderr, "can not open file \"%s\"\n", mf->p.in_file);
        return -1;
    }

    char buffer[MF_LINE_LEN];
    char ** str_array = NULL;
    int count = 0, token_size = 0;
    double score = 0.0;
    mf->mu = 0;
    mf->min_s = 1.0e10;
    mf->max_s = -1.0e10;

    IdMap *uidmap = idmap_create();
    IdMap *iidmap = idmap_create();

    while (NULL != fgets(buffer, MF_LINE_LEN, fp)){
        str_array = split(trim(buffer, 3), '\t', &count);
        if (count < 3){
            goto free_str;
        }
        if (-1 == idmap_get_value(uidmap, str_array[0])){
            idmap_add(uidmap, dupstr(str_array[0]), idmap_size(uidmap));
        }
        if (-1 == idmap_get_value(iidmap, str_array[1])){
            idmap_add(iidmap, dupstr(str_array[1]), idmap_size(iidmap));
        }
        score = atof(str_array[2]);
        mf->mu += score;
        mf->min_s = (score < mf->min_s ? score : mf->min_s);
        mf->max_s = (score > mf->max_s ? score : mf->max_s);
        token_size += 1;
free_str:
        free(str_array[0]);
        free(str_array);
    }
    mf->mu /= token_size;

    mf->U = idmap_size(uidmap);
    mf->I = idmap_size(iidmap);
    mf->T = token_size;

    malloc_train(mf);

    rewind(fp);

    int uid = -1, iid = -1, ti = 0;
    while (NULL != fgets(buffer, MF_LINE_LEN, fp)) {
        str_array = split(trim(buffer, 3), '\t', &count);
        if (count < 3){
            goto str_free;
        }
        uid = idmap_get_value(uidmap, str_array[0]);
        strncpy(mf->id_u_map[uid], str_array[0], KEY_SIZE - 1);

        iid = idmap_get_value(iidmap, str_array[1]);
        strncpy(mf->id_i_map[iid], str_array[1], KEY_SIZE - 1);

        mf->u_i[ti][0] = uid;
        mf->u_i[ti][1] = iid;
        mf->s[ti] = atof(str_array[2]);
        ti += 1;

str_free:
        free(str_array[0]);
        free(str_array);
    }
    fclose(fp);

    if (!mf->p.te_file){
        fprintf(stderr, "no test file provided\n");
        goto free_idmap;
    }

    if (NULL == (fp_t = fopen(mf->p.te_file, "r"))){
        fprintf(stderr, "test file : \"%s\" is not valid\n", mf->p.te_file);
        goto free_idmap;
    }

    token_size = 0;
    while (NULL != fgets(buffer, MF_LINE_LEN, fp_t)) {
        str_array = split(trim(buffer, 3), '\t', &count);
        if (count < 3){
            goto test_free;
        }
        token_size += 1;
test_free:
        free(str_array[0]);
        free(str_array);
    }

    mf->TT = token_size;
    malloc_test(mf);

    rewind(fp_t);

    ti = 0;
    while (NULL != fgets(buffer, MF_LINE_LEN, fp_t)){
        str_array = split(trim(buffer, 3), '\t', &count);
        if (count < 3){
            goto free_test;
        }
        uid = idmap_get_value(uidmap, str_array[0]);
        iid = idmap_get_value(iidmap, str_array[1]);
        if (uid != -1 && iid != -1){
            mf->u_t[ti][0] = uid;
            mf->u_t[ti][1] = iid;
            mf->s_t[ti] = atof(str_array[2]);
            ti += 1;
        }
free_test:
        free(str_array[0]);
        free(str_array);
    }
    fclose(fp_t);

    mf->TT = ti;

free_idmap:
    idmap_free(uidmap); uidmap = NULL;
    idmap_free(iidmap); iidmap = NULL;

    return 0;
}

void save_mf(MF * mf, int n) {
    FILE * fp = NULL;
    char bu_file[512];
    char bi_file[512];
    char pu_file[512];
    char qi_file[512];

    if (n < mf->p.niters){
        sprintf(bu_file, "%s/%d_bu",mf->p.out_dir, n);
        sprintf(bi_file, "%s/%d_bi",mf->p.out_dir, n);
        sprintf(pu_file, "%s/%d_pu",mf->p.out_dir, n);
        sprintf(qi_file, "%s/%d_qi",mf->p.out_dir, n);
    }
    else {
        sprintf(bu_file, "%s/%s_bu",mf->p.out_dir, "f");
        sprintf(bi_file, "%s/%s_bi",mf->p.out_dir, "f");
        sprintf(pu_file, "%s/%s_pu",mf->p.out_dir, "f");
        sprintf(qi_file, "%s/%s_qi",mf->p.out_dir, "f");
    }
    // output bu
    if (NULL == (fp = fopen(bu_file, "w"))){
        fprintf(stderr, "can not open file \"%s\"\n", bu_file);
        return ;
    }
    for (int i = 0; i < mf->U; i++){
        fprintf(fp, "%s\t%f\n", mf->id_u_map[i], mf->bu[i]);
    }
    fclose(fp);

    // output bi
    if (NULL == (fp = fopen(bi_file, "w"))){
        fprintf(stderr, "can not open file \"%s\"\n", bi_file);
        return ;
    }
    for (int i = 0; i < mf->I; i++){
        fprintf(fp, "%s\t%f\n", mf->id_i_map[i], mf->bi[i]);
    }
    fclose(fp);

    // output pu
    if (NULL == (fp = fopen(pu_file, "w"))){
        fprintf(stderr, "can not open file \"%s\"\n", pu_file);
        return ;
    }
    for (int i = 0; i < mf->U; i++){
        fprintf(fp, "%s", mf->id_u_map[i]);
        for (int k = 0; k < mf->p.k; k++){
            fprintf(fp, "\t%f", mf->pu[i * mf->p.k + k]);
        }
        fprintf(fp, "\n");
    }
    fclose(fp);

    // output qi
    if (NULL == (fp = fopen(qi_file, "w"))){
        fprintf(stderr, "can not open file \"%s\"\n", qi_file);
        return ;
    }
    for (int i = 0; i < mf->I; i++){
        fprintf(fp, "%s", mf->id_i_map[i]);
        for (int k = 0; k < mf->p.k; k++){
            fprintf(fp, "\t%f", mf->qi[i * mf->p.k + k]);
        }
        fprintf(fp, "\n");
    }
    fclose(fp);
}

void free_mf(MF * mf){
    if (mf){
        if (mf->id_u_map){
            free(mf->id_u_map);
            mf->id_u_map = NULL;
        }
        if (mf->id_i_map){
            free(mf->id_i_map);
            mf->id_i_map = NULL;
        }
        if (mf->u_i){
            free(mf->u_i);
            mf->u_i = NULL;
        }
        if (mf->u_t){
            free(mf->u_t);
            mf->u_t = NULL;
        }
        if (mf->s){
            free(mf->s);
            mf->s= NULL;
        }
        if (mf->s_t){
            free(mf->s_t);
            mf->s_t = NULL;
        }
        if (mf->pu_tmp){
            free(mf->pu_tmp);
            mf->pu_tmp = NULL;
        }
        if (mf->pu){
            free(mf->pu);
            mf->pu = NULL;
        }
        if (mf->qi_tmp){
            free(mf->qi_tmp);
            mf->qi_tmp = NULL;
        }
        if (mf->qi){
            free(mf->qi);
            mf->qi = NULL;
        }
        if (mf->bu){
            free(mf->bu);
            mf->bu = NULL;
        }
        if (mf->bu_tmp){
            free(mf->bu_tmp);
            mf->bu_tmp = NULL;
        }
        if (mf->bi){
            free(mf->bi);
            mf->bi = NULL;
        }
        if (mf->bi_tmp){
            free(mf->bi_tmp);
            mf->bi_tmp = NULL;
        }
        free(mf);
    }
}

