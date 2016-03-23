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
#include "hash.h"
#include "str.h"
#include "tdata.h"

#define DT_LINE_LEN 0x100000

/* -------------------------
 * brief : heap down
 * p     : index of ele down
 * l     : length of heap
 * ------------------------- */
static void heap_down(int * ids, double * vals, int p, int l){
    if (l < ((p + 1) << 1)) {
        return;
    }
    int r = (p << 1) + 1;
    int id = ids[p];
    double val = vals[p];
    do {
        if ((r < l - 1) && (vals[r] > vals[r + 1])){
            r += 1;
        }
        if (vals[p] <= vals[r]){
            break;
        }
        ids[p] = ids[r];
        vals[p] = vals[r];
        p = r;
        r += r + 1;
    }while (r < l);
    ids[p] = id;
    vals[p] = val;
}

static void heap_sort(int * ids, double * vals, int l){
    int p, id;
    double val;
    // make heap 
    p = (l - 2) >> 1;
    while (p > 0){
        heap_down(ids, vals, p, l);
        p -= 1;
    }
    heap_down(ids, vals, 0, l);
    // make sorted
    while (l > 2){     // heap with 2 ele is sorted already
        id = ids[l - 1];
        ids[l - 1] = ids[0];
        ids[0] = id;
        val  = vals[l - 1];
        vals[l - 1] = vals[0];
        vals[0] = val;
        l -= 1;
        heap_down(ids, vals, 0, l);
    }
}

static DTD * load_ds(char * input, Hash * hs, int f, int bin){
    FILE * fp = NULL;
    if (NULL == (fp = fopen(input, "r"))){
        fprintf(stderr , "can not open file \"%s\"\n", input);
        if (f == 1){
            return NULL;
        }
    }
    // Dataset Data Struct pointer
    DTD * ds = (DTD*)malloc(sizeof(DTD));
    memset(ds, 0, sizeof(DTD));
    // vals for read data
    char buffer[DT_LINE_LEN] = {'\0'};
    char *token, *string = buffer;
    int i, row, tok, offs, id, hsize;
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
    // malloc space for store data
    ds->col = hash_size(hs);
    ds->row = row;
    ds->y   = (double*)malloc(sizeof(double) * row);
    ds->l   = (int*)malloc(sizeof(int) * ds->col);
    ds->cl  = (int*)malloc(sizeof(int) * ds->col);
    memset(ds->y,  0, sizeof(double) * row);
    memset(ds->l,  0, sizeof(int) * ds->col);
    memset(ds->cl, 0, sizeof(int) * ds->col);
    for(i = 1; i < ds->col; i++){
        ds->cl[i] = fea_cnt[i - 1];
        if (i > 1){
            ds->cl[i] += ds->cl[i - 1];
        }
    }
    free(fea_cnt); fea_cnt = NULL;
    ds->ids  = (int*)malloc(sizeof(int) * tok);
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
    if (bin == 0){
        for (i = 0; i < ds->col; i++){
            if (ds->l[i] > 1){  // at least two elements
                for (int j = 0; j < ds->l[i]; j++){
                    if (ds->vals[ds->cl[i] + j] != 1.0){
                        goto sort;
                    }
                }
                // all vals equal 1.0, do not sort !!!
                continue;
sort:
                heap_sort(ds->ids + ds->cl[i], ds->vals + ds->cl[i], ds->l[i]);
            }
        }
    }
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
