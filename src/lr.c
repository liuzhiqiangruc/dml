/* ========================================================
*   Copyright (C) 2015 All rights reserved.
*
*   filename : lr.c
*   author   : lizeming@baidu.com
*   date     : 2015-08-01
*   info     : logistic regression
*              support binary or realvalued features
* ======================================================== */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "idmap.h"
#include "str.h"
#include "lr.h"

#define  LR_LINE_LEN  1048576 
#define  LR_KEY_LEN   64

void help() {
    fprintf(stderr, "\nlr [logistic regression] usage:        \n");
    fprintf(stderr, "\n./lr -f <string> -a <double> -r <int>  \n");
    fprintf(stderr, "     -f  data input file                 \n");
    fprintf(stderr, "     -a  regularized paramenter          \n");
    fprintf(stderr, "     -b  1:binary or else                \n");
    fprintf(stderr, "     -r  1:L1 Norm; 2: L2 Norm           \n\n");
}

int main(int argc, char *argv[]) {
    if ((argc & 1) == 0 || argc == 1) {
        fprintf(stderr, "command line not well formatted\n");
        help();
        return -1;
    }

    // command line parameter
    char  *arg      = NULL;    // command line 
    char  *filename = NULL;    // input file name
    int    method   = 0;       // L1, L2 method
    int    binary   = 0;       // binary feature?
    double lambda   = 0.0;     // penaty

    // parse command line
    int i = 0;
    while (i < argc) {
        arg = argv[i];
        if (0 == strcmp(arg, "-f")) {
            filename = argv[++i];
        }
        else if (0 == strcmp(arg, "-a")) {
            lambda = atof(argv[++i]);
        }
        else if (0 == strcmp(arg, "-r")) {
            method = atoi(argv[++i]);
        }
        else if (0 == strcmp(arg, "-b")) {
            binary = atoi(argv[++i]);
        }
        i += 1;
    }
    if (method != 1 && method != 2) {
        fprintf(stderr, "method must be 1, or 2.\n");
        return -1;
    }
    if (binary != 0 && binary != 1) {
        fprintf(stderr, "binary must be 0, or 1.\n");
        return -1;
    }

    // input data 
    char  (*fs)[LR_KEY_LEN] = NULL; // features
    double *retx = NULL;       // lr result
    double *y    = NULL;       // target labels
    double *val  = NULL;       // feature values for realvalued features
    int    *ids  = NULL;       // feature IDS
    int    *len  = NULL;       // feature cnt for each instance
    int  totlen  = 0;          // total input feature cnt
    int       r  = 0;          // instance cnt
    int       c  = 0;          // unique feature cnt

    // load input data
    FILE *f = NULL;
    if (NULL == (f = fopen(filename, "r"))) {
        fprintf(stderr, "can not open file \"%s\"\n", filename);
        return -1;
    }
    char buffer[LR_LINE_LEN];
    char ** str_array = NULL;
    int  col_tmp = 0;
    IdMap *idmap = idmap_create();

    // first scan for counting and idmapping
    while (NULL != fgets(buffer, LR_LINE_LEN, f)) {
        str_array = split(trim(buffer, 3), '\t', &col_tmp);
        if (col_tmp < 3){
            goto free_str;
        }
        int tlen = atoi(str_array[1]);
        if (1 == binary && col_tmp != (tlen + 2)) {
            goto free_str;
        }
        if (0 == binary && col_tmp != ((tlen + 1) << 1)) {
            goto free_str;
        }
        for (i = 0; i < tlen; i++){
            int id = 0;
            if (1 == binary){
                id = i + 2;
            }
            else {
                id = (i << 1) + 2;
            }
            if (-1 == idmap_get_value(idmap, str_array[id])){
                idmap_add(idmap, dupstr(str_array[id]), idmap_size(idmap));
            }
        }
        totlen += atoi(str_array[1]);
        r += 1; 
free_str:
        free(str_array[0]);
        free(str_array);
    }
    c  = idmap_size(idmap);
    fs = (char(*)[LR_KEY_LEN])malloc(sizeof(char[LR_KEY_LEN]) * c);
    memset(fs, 0, sizeof(char[LR_KEY_LEN]) * c);
    y    = (double*)malloc(sizeof(double) * r);
    if (0 == binary){
        val = (double*)malloc(sizeof(double) * totlen);
    }
    ids  = (int*)malloc(sizeof(int) * totlen);
    len  = (int*)malloc(sizeof(int) * r);
    rewind(f);
    // second scan for loading the data into malloced space
    totlen = r = 0;
    while (NULL != fgets(buffer, LR_LINE_LEN, f)){
        str_array = split(trim(buffer,3), '\t', &col_tmp);
        if (col_tmp < 3){
            goto str_free;
        }
        int tlen = atoi(str_array[1]);
        if (1 == binary && col_tmp != (tlen + 2)){
            goto str_free;
        }
        if (0 == binary && col_tmp != ((tlen + 1) << 1)) {
            goto str_free;
        }
        len[r] = tlen;
        y[r]   = atof(str_array[0]);
        for (i = 0; i < tlen; i++){
            int id = 0;
            if (1 == binary){
                id = i + 2;
            }
            else{
                id = (i << 1) + 2;
            }
            int iid = idmap_get_value(idmap, str_array[id]);
            ids[totlen] = iid;
            strncpy(fs[iid], str_array[id], LR_KEY_LEN - 1);   
            if (0 == binary){
                val[totlen] = atof(str_array[id + 1]);
            }
            totlen += 1;
        }
        r += 1;
str_free:
        free(str_array[0]);
        free(str_array);
    }
    fclose(f);
    // FREE the idmap
    idmap_free(idmap);
    idmap = NULL;
    // learn the coefficient
    retx = (double*)malloc(sizeof(double) * c);
    memset(retx, 0, sizeof(double) * c);
    lr(r, c, totlen, len, ids, val, y, lambda, method, retx);
    // print the learned coefficient
    for (i = 0; i < c; i++){
        printf("%s\t%.10f\n", fs[i], retx[i]);
    }
    free(y);    y    = NULL;
    free(len);  len  = NULL;
    free(ids);  ids  = NULL;
    free(retx); retx = NULL;
    if (0 == binary) {
        free(val);  val  = NULL;
    }
    return 0;
}
