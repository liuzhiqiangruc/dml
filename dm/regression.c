/* ========================================================
 *   Copyright (C) 2015 All rights reserved.
 *   
 *   filename : regression.c
 *   author   : liuzhiqiang01@baidu.com
 *   date     : 2015-08-27
 *   info     : implemention for regression
 * ======================================================== */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "str.h"
#include "idmap.h"
#include "fn_type.h"
#include "regression.h"

/* --------------------------------------------
 * brief : load train dataset 
 *         and fullfill the feature name id map
 * -------------------------------------------- */
static int load_train_ds(Regression * regression, IdMap * idmap){
    FILE * fp = NULL;
    if (NULL == (fp = fopen(regression->p.in_file, "r"))){
        fprintf(stderr, "can not open file \"%s\"\n", regression->p.in_file);
        return -1;
    }
    regression->train_ds = (RDS*)malloc(sizeof(RDS));
    memset(regression->train_ds, 0, sizeof(RDS));
    char buffer[REGRESSION_LINE_LEN];
    char ** str_array = NULL;
    int count = 0, col_cnt = 0, tf_cnt = 0, row = 0;

    // first scan for counting and idmapping
    while (NULL != fgets(buffer, REGRESSION_LINE_LEN, fp)) {
        str_array = split(trim(buffer, 3), '\t', &count);
        if (count < 3){
            goto free_str;
        }
        int tlen = atoi(str_array[1]);
        if (1 == regression->p.binary && count != (tlen + 2)){
            goto free_str;
        }
        if (0 == regression->p.binary && count != ((tlen + 1) << 1)){
            goto free_str;
        }
        for (int i = 0; i < tlen; i++){
            int id = 0;
            if (1 == regression->p.binary) id = i + 2;
            else id = (i << 1) + 2;
            if (-1 == idmap_get_value(idmap, str_array[id])) {
                idmap_add(idmap, dupstr(str_array[id]), idmap_size(idmap));
            }
        }
        tf_cnt += tlen;
        row += 1;
free_str:
        free(str_array[0]);
        free(str_array);
    }
    col_cnt = idmap_size(idmap);

    // malloc the train data space
    regression->id_map = (char(*)[REGRESSION_KEY_LEN])malloc(sizeof(char[REGRESSION_KEY_LEN]) * col_cnt);
    memset(regression->id_map, 0, sizeof(char[REGRESSION_KEY_LEN]) * col_cnt);
    regression->c = col_cnt;
    regression->x = (double*)malloc(sizeof(double) * col_cnt);
    memset(regression->x, 0, sizeof(double) * col_cnt);

    regression->train_ds->r   = row;
    regression->train_ds->y   = (double*)malloc(sizeof(double) * row);
    regression->train_ds->l   = (int*)malloc(sizeof(int) * row);
    regression->train_ds->ids = (int*)malloc(sizeof(int) * tf_cnt);
    if (0 == regression->p.binary){
        regression->train_ds->val = (double*)malloc(sizeof(double) * tf_cnt);
    }

    // second scan for loading the data to malloced space
    rewind(fp);
    tf_cnt = row = 0;
    while (NULL != fgets(buffer, REGRESSION_LINE_LEN, fp)){
        str_array = split(trim(buffer, 3), '\t', &count);
        if (count < 3) {
            goto str_free;
        }
        int tlen = atoi(str_array[1]);
        if (1 == regression->p.binary && count != (tlen + 2)){
            goto str_free;
        }
        if (0 == regression->p.binary && count != ((tlen + 1) << 1)) {
            goto str_free;
        }
        regression->train_ds->l[row] = tlen;
        regression->train_ds->y[row] = atof(str_array[0]);
        for (int i = 0; i < tlen; i++){
            int id = 0;
            if (1 == regression->p.binary) id = i + 2;
            else id = (i << 1) + 2;
            int iid = idmap_get_value(idmap, str_array[id]);
            regression->train_ds->ids[tf_cnt] = iid;
            strncpy(regression->id_map[iid], str_array[id], REGRESSION_KEY_LEN - 1);
            if (0 == regression->p.binary){
                regression->train_ds->val[tf_cnt] = atof(str_array[id + 1]);
            }
            tf_cnt += 1;
        }
        row += 1;
str_free:
        free(str_array[0]);
        free(str_array);
    }
    fclose(fp);
    return 0;
}

/* ---------------------------------
 * brief : load test data set 
 *         using feature name id map
 * --------------------------------- */
static void load_test_ds(Regression * regression, IdMap * idmap){
    FILE * fp = NULL;
    if (NULL == (fp = fopen(regression->p.te_file, "r"))){
        return;
    }
    regression->test_ds = (RDS*)malloc(sizeof(RDS));
    memset(regression->test_ds, 0, sizeof(RDS));
    char buffer[REGRESSION_LINE_LEN];
    char ** str_array = NULL;
    int count = 0, col_cnt = 0, tf_cnt = 0, row = 0;

    // first scan for counting and using train idmap
    while (NULL != fgets(buffer, REGRESSION_LINE_LEN, fp)) {
        str_array = split(trim(buffer, 3), '\t', &count);
        if (count < 3){
            goto free_str;
        }
        int tlen = atoi(str_array[1]);
        if (1 == regression->p.binary && count != (tlen + 2)){
            goto free_str;
        }
        if (0 == regression->p.binary && count != ((tlen + 1) << 1)){
            goto free_str;
        }
        int rtlen = 0;
        for (int i = 0; i < tlen; i++){
            int id = 0;
            if (1 == regression->p.binary) id = i + 2;
            else id = (i << 1) + 2;
            if (-1 != idmap_get_value(idmap, str_array[id])) {
                rtlen += 1;
            }
        }
        if (rtlen > 0){
            tf_cnt += rtlen;
            row += 1;
        }
free_str:
        free(str_array[0]);
        free(str_array);
    }

    // malloc space for test data
    regression->test_ds->r   = row;
    regression->test_ds->y   = (double*)malloc(sizeof(double) * row);
    regression->test_ds->l   = (int*)malloc(sizeof(int) * row);
    regression->test_ds->ids = (int*)malloc(sizeof(int) * tf_cnt);
    if (0 == regression->p.binary){
        regression->test_ds->val = (double*)malloc(sizeof(double) * tf_cnt);
    }

    // second scan for loading the data to malloced space
    rewind(fp);
    tf_cnt = row = 0;
    while (NULL != fgets(buffer, REGRESSION_LINE_LEN, fp)){
        str_array = split(trim(buffer, 3), '\t', &count);
        if (count < 3) {
            goto str_free;
        }
        int tlen = atoi(str_array[1]);
        if (1 == regression->p.binary && count != (tlen + 2)){
            goto str_free;
        }
        if (0 == regression->p.binary && count != ((tlen + 1) << 1)) {
            goto str_free;
        }
        int rtlen = 0;
        for (int i = 0; i < tlen; i++){
            int id = 0;
            if (1 == regression->p.binary) id = i + 2;
            else id = (i << 1) + 2;
            int iid = idmap_get_value(idmap, str_array[id]);
            if (iid != -1){
                regression->test_ds->ids[tf_cnt] = iid;
                if (0 == regression->p.binary){
                    regression->test_ds->val[tf_cnt] = atof(str_array[id + 1]);
                }
                tf_cnt += 1;
                rtlen  += 1;
            }
        }
        if (rtlen > 0) {
            regression->test_ds->l[row] = rtlen;
            regression->test_ds->y[row] = atof(str_array[0]);
            row += 1;
        }
str_free:
        free(str_array[0]);
        free(str_array);
    }
    fclose(fp);
}

/* -------------------------------
 * brief : free regression dataset
 * ------------------------------- */
void free_ds(RDS * ds){
    if (ds){
        if (ds->l){
            free(ds->l);
            ds->l = NULL;
        }
        if (ds->y){
            free(ds->y);
            ds->y = NULL;
        }
        if (ds->ids){
            free(ds->ids);
            ds->ids = NULL;
        }
        if (ds->val){
            free(ds->val);
            ds->val = NULL;
        }
        free(ds);
    }
}

Regression * create_regression(){
    Regression * regression = (Regression*)malloc(sizeof(Regression));
    memset(regression, 0, sizeof(Regression));
    return regression;
}

int init_regression(Regression * regression){
    IdMap * idmap = idmap_create();
    if (0 != load_train_ds(regression, idmap)) {
        idmap_free(idmap); idmap = NULL;
        return -1;
    }
    if (NULL != regression->p.te_file) {
        load_test_ds(regression, idmap);
    }
    idmap_free(idmap); idmap = NULL;
    return 0;
}

void save_regression(Regression * regression, int n){
    FILE * fp = NULL;
    char out_file[512];
    if (n < regression->p.niters){
        sprintf(out_file, "%s/%d_coe", regression->p.out_dir, n);
    }
    else {
        sprintf(out_file, "%s/%s_coe", regression->p.out_dir, "f");
    }
    if (NULL == (fp = fopen(out_file, "w"))){
        fprintf(stderr, "can not write to file \"%s\"\n", out_file);
        return;
    }
    for (int i = 0; i < regression->c; i++){
        fprintf(fp, "%s\t%.10f\n", regression->id_map[i], regression->x[i]);
    }
    fclose(fp);
}

void free_regression(Regression * regression){
    if (regression->train_ds){
        free_ds(regression->train_ds);
        regression->train_ds = NULL;
    }
    if (regression->test_ds){
        free_ds(regression->test_ds);
        regression->test_ds = NULL;
    }
    if (regression->x){
        free(regression->x);
        regression->x = NULL;
    }
    if (regression->id_map){
        free(regression->id_map);
        regression->id_map = NULL;
    }
    free(regression);
}
