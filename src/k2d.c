/* ========================================================
 *   Copyright (C) 2016 All rights reserved.
 *   
 *   filename : k2d.c
 *   author   : liuzhiqiangruc@126.com
 *   date     : 2016-06-28
 *   info     : 
 * ======================================================== */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "k2d.h"

void help(){
    fprintf(stderr, "k2d usage:\n");
    fprintf(stderr, "./k2d -k <int> -d <string> -o <string>\n");
}

void save(double (*cls)[6], int *ass, int n, int k, char * output){
    char name[100] = {'\0'};
    int i;
    snprintf(name, 100, "%s/assign", output);
    FILE * fp = fopen(name, "w");
    for (i = 0; i < n; i++){
        fprintf(fp, "%d\n", ass[i]);
    }
    fclose(fp);
    snprintf(name, 100, "%s/centers", output);
    fp = fopen(name, "w");
    for (i = 0; i < k; i++){
        fprintf(fp, "%.8f\t", cls[i][0]);
        fprintf(fp, "%.8f\t", cls[i][1]);
        fprintf(fp, "%.8f\t", cls[i][2]);
        fprintf(fp, "%.8f\t", cls[i][3]);
        fprintf(fp, "%.8f\t", cls[i][4]);
        fprintf(fp, "%.8f\n", cls[i][5]);
    }
    fclose(fp);
}

int main(int argc, char *argv[]){
    if (argc < 7){
        help();
        return -1;
    }
    double (*x)[2] = NULL;
    int * assign = NULL;
    double (*cls)[6] = NULL;
    int k = atoi(argv[2]);
    char * input = argv[4];
    char * output = argv[6];
    char buffer[100] = {'\0'};
    int i, n = 0;
    FILE * fp = NULL;
    if (NULL == (fp = fopen(input, "r"))){
        fprintf(stderr, "can not open input file\n");
        return -1;
    }
    while (NULL != fgets(buffer, 100, fp)){
        n += 1;
    }
    x = (double(*)[2])malloc(sizeof(double[2]) * n);
    assign = (int*)malloc(sizeof(int) * n);
    rewind(fp);
    for (i = 0; i < n; i++){
        fscanf(fp, "%lf\t%lf", x[i], x[i] + 1);
    }
    fclose(fp);
    cls = k2d(x, assign, n, k);
    save(cls, assign, n, k, output);
    free(x);         x = NULL;
    free(cls);       cls = NULL;
    free(assign);    assign = NULL;
    return 0;
}
