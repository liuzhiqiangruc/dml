/* ========================================================
 *   Copyright (C) 2015 All rights reserved.
 *   
 *   filename : breakout.c
 *   author   : ***
 *   date     : 2015-01-04
 *   info     : 
 * ======================================================== */

#include <stdio.h>
#include <stdlib.h> 
#include <string.h>
#include "edm.h"


void mm_norm(double * d, int n){
    double max = -10e10;
    double min = 10e10;
    for(int i = 0; i < n; i++){
        if (max < d[i]) max = d[i];
        if (min > d[i]) min = d[i];
    }
    for(int i = 0; i < n; i++){
        d[i] = (d[i] - min)/(max-min);
    }
}

void help(){
    fprintf(stderr, "breakout usage:\n");
    fprintf(stderr, "   ./breakout ts_file[string] min_size[int] beta[double] degree[1|2|0]\n");
    fprintf(stderr, "     ts_file   : time series file\n");
    fprintf(stderr, "     min_size  : min size between two break point\n");
    fprintf(stderr, "     beta      : penalty factor , eg: 0.001\n");
    fprintf(stderr, "     degree    : penalty degree\n");
}


int main(int argc, char * argv[]){
    if (argc < 5){
        help();
        return -1;
    }
    char * filename = argv[1];
    int min_size = atoi(argv[2]);
    double beta  = atof(argv[3]);
    int degree   = atoi(argv[4]);
    int tscnt    = 0;
    char buffer[1024];
    double *a = NULL;

    FILE *f = fopen(filename,"r");
    while(fgets(buffer, 1024, f) != NULL){
        tscnt += 1;
    }
    a = (double*)malloc(sizeof(double) * tscnt);
    memset(a, 0, sizeof(double) * tscnt);
    rewind(f);

    for (int i = 0; i < tscnt; ++i){
        fscanf(f, "%lf", a + i);
    }

    fclose(f);
    mm_norm(a, tscnt);
    int count = 0;
    int * loc = breakout(a, tscnt, min_size, beta, degree, &count);
    for (int i = 0; i < count; i++){
        printf("%d ", loc[i]);
    }
    printf("\n");
    free(loc);
    loc = NULL;
    return 0;
}
