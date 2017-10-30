/* ========================================================
 *   Copyright (C) 2016 All rights reserved.
 *   
 *   filename : tdata.c
 *   author   : liuzhiqiangruc@126.com
 *   date     : 2016-03-05
 *   info     : 
 * ======================================================== */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include "hash.h"
#include "str.h"
#include "tdata.h"

#define DT_LINE_LEN 0x100000

typedef struct {
    unsigned int id;
    double val;
} DTD_FEA_VAL;

static int tdt_fea_cmp(const void * a1, const void * a2){
    DTD_FEA_VAL * t1 = (DTD_FEA_VAL*)a1;
    DTD_FEA_VAL * t2 = (DTD_FEA_VAL*)a2;
    double t = t1->val - t2->val;
    return t > 0.0 ? -1 : (t < 0.0 ? 1 : 0);
}

static void fea_rows_sort(unsigned int * ids, double * vals, DTD_FEA_VAL * f, int n){
    int i;
    for (i = 0; i < n; i++){
        f[i].id = ids[i];
        f[i].val = vals[i];
    }
    qsort(f, n, sizeof(DTD_FEA_VAL), tdt_fea_cmp);
    for (i = 0; i < n; i++){
        ids[i] = f[i].id;
        vals[i] = f[i].val;
    }
}

static DTD * load_ds(char * input, Hash * hs, int f, int bin){
    FILE * fp = NULL;
    if (NULL == (fp = fopen(input, "r"))){
        fprintf(stderr , "can not open file \"%s\"\n", input);
        return NULL;
    }
    long t1 = time(NULL);
    // Dataset Data Struct pointer
    DTD * ds = (DTD*)malloc(sizeof(DTD));
    memset(ds, 0, sizeof(DTD));
    // vals for read data
    char buffer[DT_LINE_LEN] = {'\0'};
    char *token, *string = buffer;
    unsigned int i, row, tok, offs, id, hsize;
    int *tmp_cnt, *fea_cnt = (int*)malloc(sizeof(int) * hash_size(hs));
    memset(fea_cnt, 0, sizeof(int) * hash_size(hs));
    hsize = hash_size(hs);
    // 1th scan for counting :
    // rows , features, length of each feature, nonmissing value count
    row = tok = 0;
    while (NULL != fgets(buffer, DT_LINE_LEN, fp)){
        string = trim(buffer, 3);
        strsep(&string, "\t");
        while (NULL != (token = strsep(&string, "\t"))){
            if (f == 1){
                id = hash_add(hs, token);
                if (id >= hsize){
                    tmp_cnt = (int*)malloc(sizeof(int) * hash_size(hs));
                    memset(tmp_cnt, 0, sizeof(int) * hash_size(hs));
                    memmove(tmp_cnt, fea_cnt, sizeof(int) * hsize);
                    hsize = hash_size(hs);
                    free(fea_cnt);
                    fea_cnt = tmp_cnt;
                }
                fea_cnt[id] += 1;
                tok += 1;
            }
            else{
                id = hash_find(hs, token);
                if (id != -1){
                    fea_cnt[id] += 1;
                    tok += 1;
                }
            }
            if (bin == 0){
                strsep(&string, "\t");
            }
        }
        row += 1;
    }
    long t2 = time(NULL);
    fprintf(stderr, "first scan for feature set using : %ld seconds\n", t2 - t1);

    // malloc space for store data
    ds->col = hash_cnt(hs);
    ds->row = row;
    ds->y   = (double*)malloc(sizeof(double) * row);
    ds->l   = (unsigned int*)malloc(sizeof(int) * ds->col);
    ds->cl  = (unsigned int*)malloc(sizeof(int) * ds->col);
    memset(ds->y,  0, sizeof(double) * row);
    memset(ds->l,  0, sizeof(int) * ds->col);
    memset(ds->cl, 0, sizeof(int) * ds->col);
    for(i = 1; i < ds->col; i++){
        ds->cl[i] = fea_cnt[i - 1] + ds->cl[i - 1];
    }
    free(fea_cnt); fea_cnt = NULL;
    ds->ids  = (unsigned int*)malloc(sizeof(int) * tok);
    memset(ds->ids, 0, sizeof(int) * tok);
    if (bin == 0){
        ds->vals = (double*)malloc(sizeof(double) * tok);
        memset(ds->vals, 0, sizeof(double) * tok);
    }
    if (f == 1){
        ds->id_map = (char(*)[FKL])malloc(FKL * ds->col);
        memset(ds->id_map, 0, FKL * ds->col);
    }
    // rewind fp
    rewind(fp);
    row = 0;
    // 2nd scan to load data
    while (NULL != fgets(buffer, DT_LINE_LEN, fp)){
        string = trim(buffer, 3);
        token = strsep(&string, "\t");
        ds->y[row] = atof(token);
        while (NULL != (token = strsep(&string, "\t"))){
            id = hash_find(hs, token);
            if (f == 1 && (!ds->id_map[id][0])){
                strncpy(ds->id_map[id], token, FKL - 1);
            }
            if (id != -1){
                offs = ds->cl[id];
                ds->ids[offs + ds->l[id]] = row;
                if (bin == 0){
                    token = strsep(&string, "\t");
                    ds->vals[offs + ds->l[id]] = atof(token);
                }
                ds->l[id] += 1;
            }
        }
        row += 1;
    }
    fclose(fp);
    t1 = time(NULL);
    fprintf(stderr, "second scan load data using : %ld seconds\n", t1 - t2);

    // malloc space for store data
    if (bin == 0){
        DTD_FEA_VAL * f = (DTD_FEA_VAL*)malloc(sizeof(DTD_FEA_VAL) * row);
        for (i = 0; i < ds->col; i++){
            if (ds->l[i] > 1){  // at least two elements
                for (int j = 0; j < ds->l[i]; j++){
                    if (ds->vals[ds->cl[i] + j] != 1.0){
                        goto fea_sort;
                    }
                }
                // all vals equal 1.0, do not sort !!!
                continue;
fea_sort:
                memset(f, 0, sizeof(DTD_FEA_VAL) * row);
                fea_rows_sort(ds->ids + ds->cl[i], ds->vals + ds->cl[i], f, ds->l[i]);
            }
        }
        free(f); f = NULL;
    }
    t2 = time(NULL);
    fprintf(stderr, "sort features using : %ld seconds\n", t2 - t1);

    ds->bin = bin;
    return ds;
}

DTD *(*load_data(char * train_input, char * test_input, int binary))[2]{
    if (!train_input){
        return NULL;
    }
    Hash * hs = hash_create(1 << 20, STRING);
    DTD * train_ds = load_ds(train_input, hs, 1, binary);
    if (!train_ds){
        fprintf(stderr, "load train data failed\n");
        return NULL;
    }
    DTD * test_ds = load_ds(test_input, hs, 0, binary);
    if (!test_ds){
        fprintf(stderr, "no test data or read failed\n");
    }
    hash_free(hs); hs = NULL;
    DTD *(*ds)[2] = (DTD*(*)[2])malloc(sizeof(void *) * 2);
    (*ds)[0] = train_ds;
    (*ds)[1] = test_ds;
    return ds;
}

void free_data(DTD *ts){
    if (ts){
        if (ts->y){
            free(ts->y);
            ts->y = NULL;
        }
        if (ts->l){
            free(ts->l);
            ts->l = NULL;
        }
        if (ts->cl){
            free(ts->cl);
            ts->cl = NULL;
        }
        if (ts->ids){
            free(ts->ids);
            ts->ids = NULL;
        }
        if (ts->vals){
            free(ts->vals);
            ts->vals = NULL;
        }
        if (ts->id_map){
            free(ts->id_map);
            ts->id_map = NULL;
        }
        free(ts);
    }
}
