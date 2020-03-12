/* ========================================================
 *   Copyright (C) 2016 All rights reserved.
 *   
 *   filename : kmeans.c
 *   author   : ***
 *   date     : 2016-11-22
 *   info     : kmeans++ exec binary, 
 *              load data and pretrained cents
 * ======================================================== */

#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "kmeans.h"

void help(){
    fprintf(stderr, "kmeans usage:\n");
    fprintf(stderr, "./kmeans -f [int] -k [int] -m [int] -n [int] -e [double] -d [string] -p [string] -o [string]\n");
    fprintf(stderr, "         -f dim\n");
    fprintf(stderr, "         -k max cluster\n");
    fprintf(stderr, "         -m thread count\n");
    fprintf(stderr, "         -n iteration\n");
    fprintf(stderr, "         -e min value for instance to be a new cluster\n");
    fprintf(stderr, "         -d input embeddings\n");
    fprintf(stderr, "         -p init centers\n");
    fprintf(stderr, "         -o output dir\n");
}

int main(int argc, char *argv[]){
    if (argc < 15){
        help();
        return -1;
    }
    int f, k, n, i, t, maxn, initk;
    int *c = NULL;
    double inithe;
    double *m = NULL, *dist = NULL, *cents = NULL;
    char *inf = NULL, *o = ".", *initc = NULL;
    char buffer[4096] = {0};
    char of[256] = {0};
    FILE * fp = NULL;
    f = atoi(argv[2]);
    k = atoi(argv[4]);
    t = atoi(argv[6]);
    maxn = atoi(argv[8]);
    inithe = atof(argv[10]);
    inf = argv[12];
    initc = argv[14];
    if (argc == 17){
        o = argv[16];
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

    initk = 0;
    if (NULL == (fp = fopen(initc, "r"))){
        fprintf(stderr, "can not open the inited centers, reboot clustering\n");
        goto train_process;
    }
    while(NULL != fgets(buffer, 4096, fp)){
        initk += 1;
    }
    if (initk > k){
        fprintf(stderr, " init error\n");
        fclose(fp);
        goto ret;
    }
    rewind(fp);
    i = 0;
    fscanf(fp, "%lf", cents);
    while(!feof(fp)){
        fscanf(fp, "%lf", cents + (++i));
    }
    fclose(fp);
    if (i != initk * f){
        goto ret;
    }

train_process:
    k = kmeans(m, n, f, k, initk, cents, c, dist, t, maxn, inithe);

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
            fprintf(fp, "\t%.6f", cents[i * f + j]);
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
