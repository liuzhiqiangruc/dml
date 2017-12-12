/* ========================================================
 *   Copyright (C) 2017 All rights reserved.
 *   
 *   filename : data.c
 *   author   : ***
 *   date     : 2017-09-23
 *   info     : 
 * ======================================================== */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#include "hash.h"
#include "data.h"
#include "str.h"

#define DT_LINE_LEN 0x100000

typedef struct {
    unsigned int id;
    double val;
} DATA_FEA_VAL;

static int tdt_fea_cmp(const void * a1, const void * a2){
    DATA_FEA_VAL * t1 = (DATA_FEA_VAL*)a1;
    DATA_FEA_VAL * t2 = (DATA_FEA_VAL*)a2;
    double t = t1->val - t2->val;
    return t > 0.0 ? -1 : (t < 0.0 ? 1 : 0);
}

static void fea_rows_sort(unsigned int * ids, double * vals, DATA_FEA_VAL * f, int n){
    int i;
    for (i = 0; i < n; i++){
        f[i].id = ids[i];
        f[i].val = vals[i];
    }
    qsort(f, n, sizeof(DATA_FEA_VAL), tdt_fea_cmp);
    for (i = 0; i < n; i++){
        ids[i] = f[i].id;
        vals[i] = f[i].val;
    }
}

static int stat_data_input(const char * input, DATA * ds, Hash * hs, int ** fea_cnt_p){
    unsigned int row, id, tok, hsize = 0;
    int *tmp_cnt = NULL, *fea_cnt = NULL;
    FILE * fp = NULL;
    char buffer[DT_LINE_LEN] = {0};
    char *token, *string = buffer;
    if (NULL == (fp = fopen(input, "r"))){
        fprintf(stderr , "can not open file \"%s\"\n", input);
        return -1;
    }
    if (ds->dt_style == COL){
        hsize = hash_size(hs);
        fea_cnt = (int*)malloc(sizeof(int) * hsize);
        memset(fea_cnt, 0, sizeof(int) * hsize);
    }
    row = tok = 0;
    while (NULL != fgets(buffer, DT_LINE_LEN, fp)){
        string = trim(buffer, 3);
        strsep(&string, "\t");
        while (NULL != (token = strsep(&string, "\t"))){
            if (ds->fea_set == NO_INITED){ // load data without given feature set
                id = hash_add(hs, token);
                if (ds->dt_style == COL){
                    if (id >= hsize){
                        tmp_cnt = (int*)malloc(sizeof(int) * hash_size(hs));
                        memset(tmp_cnt, 0, sizeof(int) * hash_size(hs));
                        memmove(tmp_cnt, fea_cnt, sizeof(int) * hsize);
                        hsize = hash_size(hs);
                        free(fea_cnt);
                        fea_cnt = tmp_cnt;
                    }
                    fea_cnt[id] += 1;
                }
                tok += 1;
            }
            else {    // load data with given feature set
                id = hash_find(hs, token);
                if (id != -1){
                    if (ds->dt_style == COL){
                        fea_cnt[id] += 1;
                    }
                    tok += 1;
                }
            }
            if (ds->fea_type == NOBINARY){
                strsep(&string, "\t");
            }
        }
        row += 1;
    }
    ds->col    = hash_cnt(hs);
    ds->row    = row;
    ds->tkn    = tok;
    *fea_cnt_p = fea_cnt;
    fclose(fp);
    return 0;
}

int init_data_struct(DATA * ds, int * fea_cnt){
    int i;
    if (ds->dt_style == COL && NULL == fea_cnt){
        return -1;
    }
    ds->y   = (double*)malloc(sizeof(double) * ds->row);
    memset(ds->y,  0, sizeof(double) * ds->row);
    ds->ids = (unsigned int *)malloc(sizeof(unsigned int) * ds->tkn);
    memset(ds->ids, 0, sizeof(unsigned int) * ds->tkn);
    if (ds->dt_style == COL){
        ds->len  = (unsigned int *)malloc(sizeof(unsigned int) * ds->col);
        ds->clen = (unsigned int *)malloc(sizeof(unsigned int) * ds->col);
        memset(ds->len,  0, sizeof(unsigned int) * ds->col);
        memset(ds->clen, 0, sizeof(unsigned int) * ds->col);
        for (i = 1; i < ds->col; i++){
            ds->clen[i] = fea_cnt[i - 1] + ds->clen[i - 1];
        }
        free(fea_cnt);
    }
    else{
        ds->len  = (unsigned int *)malloc(sizeof(unsigned int) * ds->row);
        ds->clen = (unsigned int *)malloc(sizeof(unsigned int) * ds->row);
        memset(ds->len,  0, sizeof(unsigned int) * ds->row);
        memset(ds->clen, 0, sizeof(unsigned int) * ds->row);
    }
    if (ds->fea_type == NOBINARY){
        ds->vals = (double*)malloc(sizeof(double) * ds->tkn);
        memset(ds->vals, 0, sizeof(double) * ds->tkn);
    }
    if (ds->fea_set == NO_INITED){
        ds->id_map = (char(*)[FKL])malloc(FKL * ds->col);
        memset(ds->id_map, 0, FKL * ds->col);
    }
    return 0;
}

int fullfill_data_struct(const char * input, DATA * ds, Hash * hs){
    int row, offs, tk, id;
    FILE * fp = NULL;
    char buffer[DT_LINE_LEN] = {0};
    char *token, *string = buffer;
    if (NULL == (fp = fopen(input, "r"))){
        fprintf(stderr , "can not open file \"%s\"\n", input);
        return -1;
    }
    row = tk = 0;
    while (NULL != fgets(buffer, DT_LINE_LEN, fp)){
        string = trim(buffer, 3);
        token = strsep(&string,"\t");
        ds->y[row] = atof(token);
        while (NULL != (token = strsep(&string, "\t"))){
            id = hash_find(hs, token);
            if (id != -1){
                if (ds->fea_set == NO_INITED && !ds->id_map[id][0]){
                    strncpy(ds->id_map[id], token, FKL - 1);
                }
                if (ds->dt_style == COL){
                    offs = ds->clen[id];
                    ds->ids[offs + ds->len[id]] = row;
                    if (ds->fea_type == NOBINARY){
                        token = strsep(&string, "\t");
                        ds->vals[offs + ds->len[id]] = atof(token);
                    }
                    ds->len[id] += 1;
                }
                else{
                    ds->ids[tk] = id;
                    if (ds->fea_type == NOBINARY){
                        token = strsep(&string, "\t");
                        ds->vals[tk] = atof(token);
                    }
                    ds->len[row] += 1;
                }
                tk += 1;
            }
        }
        row += 1;
    }
    if (ds->dt_style == ROW){
        ds->clen[0] = 0;
        for (row = 1; row < ds->row; row++){
            ds->clen[row] = ds->clen[row - 1] + ds->len[row - 1];
        }
    }
    fclose(fp);
    return 0;
}

static void pre_sort_ds(DATA *ds){
    int i, j;
    DATA_FEA_VAL * f = (DATA_FEA_VAL*)malloc(sizeof(DATA_FEA_VAL) * ds->row);
    for (i = 0; i < ds->col; i++){
        if (ds->len[i] > 1){
            for (j = 0; j < ds->len[i]; j++){
                if (ds->vals[ds->clen[i] + j] != 1.0){
                    goto fea_sort;
                }
            }
            continue; // all vals equal 1.0, do not sort !!!
fea_sort:
            memset(f, 0, sizeof(DATA_FEA_VAL) * ds->row);
            fea_rows_sort(ds->ids + ds->clen[i], ds->vals + ds->clen[i], f, ds->len[i]);
        }
    }
    free(f); f = NULL;
}

DATA * data_load(const char * input, DT_STYLE dt_style, FEA_TYPE fea_type, FEA_SET fea_set, Hash * hs){
    if (!input){
        return NULL;
    }
    // ds : dataset
    DATA * ds = (DATA*)malloc(sizeof(DATA));
    memset(ds, 0, sizeof(DATA));
    ds->dt_style = dt_style;
    ds->fea_type = fea_type;
    ds->fea_set  = fea_set;

    // first scan for stat input data: col/row/tkn
    // and load feature hash set if need
    int * fea_cnt = NULL;
    long t1 = time(NULL);
    stat_data_input(input, ds, hs, &fea_cnt);
    long t2 = time(NULL);
    fprintf(stderr, "first scan for feature set using : %ld seconds\n", t2 - t1);

    // malloc the data space 
    // and init
    if (0 != init_data_struct(ds, fea_cnt)){
        return NULL;
    }

    // second scan for load data to malloced space
    fullfill_data_struct(input, ds, hs);
    t1 = time(NULL);
    fprintf(stderr, "second scan load data using : %ld seconds\n", t1 - t2);

    // pre sort the column based and nobinarydata 
    if (ds->dt_style == COL && ds->fea_type == NOBINARY) {
        pre_sort_ds(ds);
        t2 = time(NULL);
        fprintf(stderr, "sort features using : %ld seconds\n", t2 - t1);
    }

    return ds;
}

void data_free(DATA * ds){
    if (ds){
        if (ds->len){
            free(ds->len);
            ds->len = NULL;
        }
        if (ds->clen){
            free(ds->clen);
            ds->clen = NULL;
        }
        if (ds->ids){
            free(ds->ids);
            ds->ids = NULL;
        }
        if (ds->vals){
            free(ds->vals);
            ds->vals = NULL;
        }
        if (ds->y){
            free(ds->y);
            ds->y = NULL;
        }
        if (ds->id_map){
            free(ds->id_map);
            ds->id_map = NULL;
        }
        free(ds);
    }
}
