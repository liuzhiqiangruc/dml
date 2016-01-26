/* ========================================================
 *   Copyright (C) 2016 All rights reserved.
 *   
 *   filename : regress.c
 *   author   : liuzhiqiangruc@126.com
 *   date     : 2016-01-08
 *   info     : regression framework implementation
 * ======================================================== */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "str.h"
#include "hash.h"
#include "newton.h"
#include "regress.h"

#define REG_LINE_LEN 0x100000      // 1MByte

static int load_train_ds(REG * reg, Hash * hs){
    FILE * fp = NULL;
    if (NULL == (fp = fopen(reg->p.in_file, "r"))){
        fprintf(stderr, "can not open file \"%s\"\n", reg->p.in_file);
        return -1;
    }
    reg->train_ds = (RDS*)malloc(sizeof(RDS));
    if (!reg->train_ds){
        goto train_ds_failed;
    }
    memset(reg->train_ds, 0, sizeof(RDS));
    char buffer[REG_LINE_LEN] = {'\0'};
    int col_cnt = 0, tf_cnt = 0, row = 0;
    char *token = NULL, *string = buffer;
    while (NULL != fgets(buffer, REG_LINE_LEN, fp)){
        string = trim(buffer, 3);
        token = strsep(&string, "\t");
        token = strsep(&string, "\t");
        tf_cnt += atoi(token);
        while (NULL != (token = strsep(&string, "\t"))){
            hash_add(hs, token);
            if (reg->p.binary == 0){
                strsep(&string, "\t");
            }
        }
        row += 1;
    }
    col_cnt = hash_cnt(hs);
    reg->c = col_cnt;
    reg->id_map = (char(*)[RKL])malloc(sizeof(char[RKL]) * col_cnt);
    if (!reg->id_map){
        goto id_map_failed;
    }
    memset(reg->id_map, 0, sizeof(char[RKL]) * col_cnt);
    reg->x = (double*)malloc(sizeof(double) * reg->c);
    if (!reg->x){
        goto x_failed;
    }
    memset(reg->x, 0, sizeof(double) * reg->c);
    reg->train_ds->r = row;
    reg->train_ds->y = (double*)malloc(sizeof(double) * row);
    if (!reg->train_ds->y){
        goto train_y_failed;
    }
    reg->train_ds->l = (int*)malloc(sizeof(int) * row);
    if (!reg->train_ds->l){
        goto train_l_failed;
    }
    reg->train_ds->ids = (int*)malloc(sizeof(int) * tf_cnt);
    if (!reg->train_ds->ids){
        goto train_ids_failed;
    }
    if (0 == reg->p.binary){
        reg->train_ds->val = (double*)malloc(sizeof(double) * tf_cnt);
        if(!reg->train_ds->val){
            goto train_val_failed;
        }
    }
    rewind(fp);
    tf_cnt = row = 0;
    while (NULL != fgets(buffer, REG_LINE_LEN, fp)){
        string = trim(buffer, 3);
        token = strsep(&string, "\t");
        reg->train_ds->y[row] = atof(token);
        token = strsep(&string, "\t");
        reg->train_ds->l[row] = atoi(token);
        while (NULL != (token = strsep(&string, "\t"))){
            int id = hash_find(hs, token);
            reg->train_ds->ids[tf_cnt] = id;
            strncpy(reg->id_map[id], token, RKL - 1);
            if (reg->p.binary == 0){
                token = strsep(&string, "\t");
                reg->train_ds->val[tf_cnt] = atof(token);
            }
            tf_cnt += 1;
        }
        row += 1;
    }
    fclose(fp);
    return 0;
train_val_failed:
    free(reg->train_ds->ids);
train_ids_failed:
    free(reg->train_ds->l);
train_l_failed:
    free(reg->train_ds->y);
train_y_failed:
    free(reg->x);
x_failed:
    free(reg->id_map);
id_map_failed:
    free(reg->train_ds);
train_ds_failed:
    return -1;
}

static int load_test_ds(REG * reg, Hash * hs){
    FILE * fp = NULL;
    if (NULL == (fp = fopen(reg->p.te_file, "r"))){
        return -1;
    }
    char buffer[REG_LINE_LEN] = {'\0'};
    char *string = NULL, *token = NULL;
    int tf_cnt = 0, row = 0;
    while (NULL != fgets(buffer, REG_LINE_LEN, fp)){
        string = trim(buffer, 3);
        token = strsep(&string, "\t");
        token = strsep(&string, "\t");
        int tlen = 0;
        while (NULL != (token = strsep(&string, "\t"))){
            if (-1 != hash_find(hs, token)){
                tlen += 1;
            }
            if (0 == reg->p.binary){
                strsep(&string, "\t");
            }
        }
        if (tlen > 0){
            tf_cnt += tlen;
            row += 1;
        }
    }
    if (row == 0){
        reg->test_ds = NULL;
        goto empty_test;
    }
    reg->test_ds = (RDS*)malloc(sizeof(RDS));
    if (!reg->test_ds){
        goto ds_failed;
    }
    memset(reg->test_ds, 0, sizeof(RDS));
    reg->test_ds->r = row;
    reg->test_ds->y = (double*)malloc(sizeof(double) * row);
    if (!reg->test_ds->y){
        goto test_y_failed;
    }
    memset(reg->test_ds->y, 0, sizeof(double) * row);
    reg->test_ds->l = (int*)malloc(sizeof(int) * row);
    if (!reg->test_ds->l){
        goto test_l_failed;
    }
    memset(reg->test_ds->l, 0, sizeof(int) * row);
    reg->test_ds->ids = (int*)malloc(sizeof(int) * tf_cnt);
    if (!reg->test_ds->ids){
        goto test_ids_failed;
    }
    memset(reg->test_ds->ids, 0, sizeof(int) * tf_cnt);
    if (0 == reg->p.binary){
        reg->test_ds->val = (double *)malloc(sizeof(double) * tf_cnt);
        if (!reg->test_ds->val){
            goto test_val_failed;
        }
        memset(reg->test_ds->val, 0, sizeof(double) * tf_cnt);
    }
    rewind(fp);
    tf_cnt = row = 0;
    while (NULL != fgets(buffer, REG_LINE_LEN, fp)){
        string = trim(buffer, 3);
        token = strsep(&string, "\t");
        reg->test_ds->y[row] = atof(token);
        token = strsep(&string, "\t");
        int tlen = 0;
        int id = -1;
        while (NULL != (token = strsep(&string, "\t"))){
            if (-1 != (id = hash_find(hs, token))){
                tlen += 1;
                reg->test_ds->ids[tf_cnt] = id;
            }
            if (0 == reg->p.binary){
                token = strsep(&string, "\t");
                if (id != -1){
                    reg->test_ds->val[tf_cnt] = atof(token);
                }
            }
            if (id != -1){
                tf_cnt += 1;
            }
        }
        if (tlen > 0){
            reg->test_ds->l[row] = tlen;
            row += 1;
        }
    }
    fclose(fp);
empty_test:
    return 0;
test_val_failed:
    free(reg->test_ds->ids);
test_ids_failed:
    free(reg->test_ds->l);
test_l_failed:
    free(reg->test_ds->y);
test_y_failed:
    free(reg->test_ds);
ds_failed:
    return -1;
}

static void free_ds(RDS * ds){
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

REG * create_model(EVAL_FN eval_fn, GRAD_FN grad_fn, REPO_FN repo_fn){
    if (!eval_fn || !grad_fn){
        return NULL;
    }
    REG * reg = (REG*)malloc(sizeof(REG));
    if (!reg){
        return NULL;
    }
    memset(reg, 0, sizeof(REG));
    reg->eval_fn = eval_fn;
    reg->grad_fn = grad_fn;
    reg->repo_fn = repo_fn;
    return reg;
}

int init_model(REG * reg){
    Hash * hs = hash_create(1<<20, STRING);
    if (0 != load_train_ds(reg, hs)){
        hash_free(hs); hs = NULL;
        return -1;
    }
    if (NULL != reg->p.te_file){
        load_test_ds(reg, hs);
    }
    hash_free(hs); hs = NULL;
    return 0;
}

int learn_model(REG * reg){
    NT_METHOD method;
    if (reg->p.method == 1){
        method = OWLQN;
    }
    else if(reg->p.method == 2) {
        method = LBFGS;
    }
    else{
        return -1;
    }
    newton(reg->eval_fn, reg->grad_fn, reg->repo_fn, method, reg, 5, reg->p.niters, reg->c, reg->x);
    return 0;
}

void save_model(REG * reg, int n){
    FILE * fp = NULL;
    char out_file[512] = {'\0'};
    if (n < reg->p.niters){
        sprintf(out_file, "%s/%d_coe", reg->p.out_dir, n);
    }
    else{
        sprintf(out_file, "%s/%s_coe", reg->p.out_dir, "f");
    }
    if (NULL == (fp = fopen(out_file, "w"))){
        fprintf(stderr, "can not open file \"%s\"\n", out_file);
        return;
    }
    for (int i = 0; i < reg->c; i++){
        fprintf(fp, "%s\t%.10f\n", reg->id_map[i], reg->x[i]);
    }
    fclose(fp);
}

void free_model(REG * reg){
    if (reg->train_ds){
        free_ds(reg->train_ds);
        reg->train_ds = NULL;
    }
    if (reg->test_ds){
        free(reg->test_ds);
        reg->test_ds = NULL;
    }
    if (reg->x){
        free(reg->x);
        reg->x = NULL;
    }
    if (reg->id_map){
        free(reg->id_map);
        reg->id_map = NULL;
    }
    free(reg);
}

