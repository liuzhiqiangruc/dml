/* ========================================================
 *   Copyright (C) 2016 All rights reserved.
 *   
 *   filename : kmeans.c
 *   author   : ***
 *   date     : 2016-11-22
 *   info     : 
 * ======================================================== */

#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "kmeans.h"

void help(){
    fprintf(stderr, "kmeans usage:\n");
    fprintf(stderr, "./kmeans -f [int] -k [int] -d [string] -o [string]\n");
}

int main(int argc, char *argv[]){
    if (argc < 7){
        help();
        return -1;
    }
    srand(time(NULL));
    int f, k, n, i;
    int *c = NULL;
    double *m = NULL;
    char *inf = NULL, *o = ".";
    char buffer[4096] = {0};
    char of[256] = {0};
    FILE * fp = NULL;
    f = atoi(argv[2]);
    k = atoi(argv[4]);
    inf = argv[6];
    if (argc == 9){
        o = argv[8];
    }
    if (NULL == (fp = fopen(inf, "r"))){
        fprintf(stderr, "can not open input file\n");
        return -1;
    }
    n = 0;
    while (NULL != fgets(buffer, 4096, fp)){
        n += 1;
    }
    rewind(fp);

    m = (double*)calloc(sizeof(double), n * f);
    c = (int*)calloc(sizeof(int), n);
    
    i = 0;
    fscanf(fp, "%lf", m);
    while (!feof(fp)){
        fscanf(fp, "%lf", m + (++i));
    }
    fclose(fp);
    
    if (i != n * f){
        goto ret;
    }

    kmeans(m, n, f, k, c, 32);

    sprintf(of, "%s/clsid", o);
    
    fp = fopen(of, "w");
    if (fp == NULL){
        fp = stdout;
    }

    i = 0;
    while (i < n){
        fprintf(fp, "%d\n", c[i++]);
    }

    if (fp != stdout){
        fclose(fp);
    }


ret:
    free(m);
    free(c);

    return 0;
}
