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

/* ------------------------------------
 * brief : heap_up
 * l     : index of ele need to heap up
 * ------------------------------------ */
static void heap_up(DPair * h, int l){
    int p;
    DPair t = h[l];
    while (l > 0){
        p = (l - 1) >> 1;
        if (h[p].val > h[l].val){
            h[l] = h[p];
        }
        l = p;
    }
    h[l] = t;
}

/* ----------------------
 * brief : heap down
 * l     : length of h
 * ---------------------- */
static void heap_down(DPair * h, int l){
    if (l < 2){
        return;
    }
    int p = 0, r = 1;
    DPair d = h[0];
    do {
        if ((r < l - 1) && (h[r].val > h[r + 1].val)){
            r += 1;
        }
        if (h[p].val <= h[r].val){
            break;
        }
        h[p] = h[r];
        p = r;
        r += r + 1;
    }while (r < l);
    h[p] = d;
}

static void heap_sorted(DPair * h, int l){
    int i, t;
    DPair d;
    do {
        d = h[l - 1];
        h[l - 1] = h[0];
        h[0] = d;
        l -= 1;
        heap_down(h, l);
    } while (l > 1);
}

static DTD * load_ds(char * input, Hash * hs, int f){
    FILE * fp = NULL;
    if (NULL == (fp = fopen(input, "r"))){
        fprintf(stderr , "can not open file \"%s\"\n", input);
        if (f == 1){
            return NULL;
        }
    }

    DTD * ds = (DTD*)malloc(sizeof(DTD));
    memset(ds, 0, sizeof(DTD));

    char buffer[DT_LINE_LEN] = {'\0'};
    char *token, *string = buffer;
    int i, row, tok, offs, id, hsize;
    int *tmp_cnt, *fea_cnt = (int*)malloc(sizeof(int) * hash_size(hs));
    memset(fea_cnt, 0, sizeof(int) * hash_size(hs));
    hsize = hash_size(hs);

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
            strsep(&string, "\t");
        }
        row += 1;
    }

    ds->col = hash_size(hs);
    ds->row = row;
    ds->y   = (double*)malloc(sizeof(double) * row);
    memset(ds->y, 0, sizeof(double) * row);
    ds->l   = (int*)malloc(sizeof(int) * ds->col);
    ds->cl  = (int*)malloc(sizeof(int) * ds->col);
    memset(ds->l,  0, sizeof(int) * ds->col);
    memset(ds->cl, 0, sizeof(int) * ds->col);
    for(i = 1; i < ds->col; i++){
        ds->cl[i] = fea_cnt[i - 1];
        if (i > 1){
            ds->cl[i] += ds->cl[i - 1];
        }
    }
    free(fea_cnt); fea_cnt = NULL;
    ds->vals = (DPair*)malloc(sizeof(DPair) * tok);
    memset(ds->vals, 0, sizeof(DPair) * tok);
    if (f == 1){
        ds->id_map = (char(*)[FKL])malloc(FKL * ds->col);
        memset(ds->id_map, 0, FKL * ds->col);
    }

    rewind(fp);

    row = 0;
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
                ds->vals[offs + ds->l[id]].id = row;
                token = strsep(&string, "\t");
                ds->vals[offs + ds->l[id]].val = atof(token);
                heap_up(ds->vals + offs, ds->l[id]);
                ds->l[id] += 1;
            }
        }
        row += 1;
    }
    fclose(fp);

    for (i = 0; i < ds->col; i++){
        DPair * d = ds->vals + ds->cl[i];
        heap_sorted(d, ds->l[i]);
    }

    return ds;
}


DTD *(*load_data(char * train_input, char * test_input))[2]{
    if (!train_input){
        return NULL;
    }
    Hash * hs = hash_create(0x100000, STRING);
    DTD * train_ds = load_ds(train_input, hs, 1);
    if (!train_ds){
        fprintf(stderr, "load train data failed\n");
        return NULL;
    }
    DTD * test_ds = load_ds(test_input, hs, 0);
    if (!test_ds){
        fprintf(stderr, "no test data or read failed\n");
    }
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
