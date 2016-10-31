/* ========================================================
 *   Copyright (C) 2016 All rights reserved.
 *   
 *   filename : tdata.c
 *   author   : liuzhiqiangruc@126.com
 *   date     : 2016-10-27
 *   info     : 
 * ======================================================== */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "str.h"
#include "hash.h"
#include "tsdata.h"

#define LINE_LEN 1024

TSD * tsd_load(char * infile) {
    FILE * fp = NULL;
    if (NULL == (fp = fopen(infile, "r"))){
        fprintf(stderr, "can not open input file\n");
        return NULL;
    }
    int  id, tk = 0, dsize = 0;
    char buffer[LINE_LEN] = {'\0'};
    char *string = NULL, *token = NULL;
    Hash * whs = hash_create(1<<20, STRING);
    TSD * ds = (TSD*)calloc(1, sizeof(TSD));
    while (NULL != fgets(buffer, LINE_LEN, fp)){
        string = trim(buffer, 3);
        while (NULL != (token = strsep(&string, "\t"))){
            hash_add(whs, token);
            tk += 1;
        }
        dsize += 1;
    }
    ds->d = dsize;
    ds->v = hash_cnt(whs);
    ds->t = tk;
    ds->doffs  = (int*)calloc(ds->d + 1,  sizeof(int));
    ds->tokens = (int*)calloc(ds->t,      sizeof(int));
    ds->fcnt   = (int(*)[2])calloc(ds->v, sizeof(int[2]));
    ds->idm    = (char(*)[KEY_SIZE])calloc(ds->v, sizeof(char[KEY_SIZE]));
    rewind(fp);
    dsize = tk = 0;
    while (NULL != fgets(buffer, LINE_LEN, fp)){
        string = trim(buffer, 3);
        while(NULL != (token = strsep(&string, "\t"))){
            id = hash_find(whs, token);
            ds->tokens[tk++] = id;
            ds->fcnt[id][0] = id;
            ds->fcnt[id][1] += 1;
            if (ds->idm[id][0] == '\0'){
                strncpy(ds->idm[id], token, KEY_SIZE - 1);
            }
        }
        ds->doffs[++dsize] = tk;
    }
    fclose(fp);
    free(whs); whs = NULL;
    return ds;
}

void tsd_free(TSD * ds){
    if (ds){
        if (ds->doffs){
            free(ds->doffs);
            ds->doffs = NULL;
        }
        if (ds->tokens){
            free(ds->tokens);
            ds->tokens = NULL;
        }
        if (ds->fcnt){
            free(ds->fcnt);
            ds->fcnt = NULL;
        }
        if (ds->idm){
            free(ds->idm);
            ds->idm = NULL;
        }
        free(ds);
    }
}
