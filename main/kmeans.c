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
    fprintf(stderr, "./kmeans -f [int] -k [int] -m [int] -d [string] -o [string]\n");
}

int main(int argc, char *argv[]){
    if (argc < 9){
        help();
        return -1;
    }
    srand(time(NULL));
    int f, k, n, i, t;
    int *c = NULL;
    double *m = NULL, *dist = NULL, *cents = NULL;
    char *inf = NULL, *o = ".";
    char buffer[4096] = {0};
    char of[256] = {0};
    FILE * fp = NULL;
    f = atoi(argv[2]);
    k = atoi(argv[4]);
    t = atoi(argv[6]);
    inf = argv[8];
    if (argc == 11){
        o = argv[10];
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

    m     = (double*)calloc(sizeof(double), n * f);
    cents = (double*)calloc(sizeof(double), k * f);
    c     = (int*)calloc(sizeof(int), n);
    dist  = (double*)calloc(sizeof(double), n);
    
    i = 0;
    fscanf(fp, "%lf", m);
    while (!feof(fp)){
        fscanf(fp, "%lf", m + (++i));
    }
    fclose(fp);
    
    if (i != n * f){
        goto ret;
    }

    kmeans(m, n, f, k, cents, c, dist, t);

    // output the item clusid
    sprintf(of, "%s/clsid", o);
    fp = fopen(of, "w");
    if (fp == NULL){
        fp = stdout;
    }
    i = 0;
    while (i < n){
        fprintf(fp, "%d\t%.6f\n", c[i], dist[i]);
        i += 1;
    }
    if (fp != stdout){
        fclose(fp);
    }
    // output the cluster centers
    sprintf(of, "%s/cents", o);
    fp = fopen(of, "w");
    if (fp == NULL) fp = stdout;
    i = 0;
    while (i < k){
        fprintf(fp, "cluster_%04d", i);
        for (int j = 0; j < f; j++){
            fprintf(fp, "\t%.6f", cents[i * k + j]);
        }
        fprintf(fp, "\n");
        i += 1;
    }
    if (fp != stdout) fclose(fp);


ret:
    free(m);
    free(c);
    free(dist);
    free(cents);

    return 0;
}
