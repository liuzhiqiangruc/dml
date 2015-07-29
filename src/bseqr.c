/* ========================================================
 *   Copyright (C) 2015 All rights reserved.
 *   
 *   filename : bseqr.c
 *   author   : ***
 *   date     : 2015-04-29
 *   info     : 
 * ======================================================== */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "modl.h"


void help(){
    fprintf(stderr, "\nbinomial sequence regularization usage: \n");
    fprintf(stderr, "./bseqr -f <string> -n <int>\n\n");
    fprintf(stderr, "        -f input seq file one value per line \n");
    fprintf(stderr, "        -n line num of input seq file\n\n");
}

int main(int argc, char * argv[]){
    if (argc < 5){
        help();
        return -1;
    }
    char * input = NULL;
    char * arg = NULL;
    int n = 0;
    int i = 0;
    while (i < argc){
        arg = argv[i];
        if (0 == strcmp(arg, "-f")){
            input = argv[++i];
        }
        else if (0 == strcmp(arg, "-n")){
            n = atoi(argv[++i]);
        }
        i += 1;
    }
    if (NULL ==  input){
        fprintf(stderr,"no input data exit(-1)\n");
        return -1;
    }
    FILE * f = NULL;
    if (NULL == (f = fopen(input,"r"))){
        fprintf(stderr, "can not open input file \n");
        return -1;
    }

    int * l = (int*)malloc(sizeof(int) * n);
    double * d = (double*)malloc(sizeof(double) * n);
    memset(l, 0, sizeof(int) * n);
    memset(d, 0, sizeof(double) * n);

    for (int i = 0; i < n; i++){
        d[i] = (double)i;
        fscanf(f, "%d", l + i);
    }
    fclose(f);

    int nd = 0;
    double * LogD = initLogD(n << 1);
    double * sp = modl(d, l, LogD, n, &nd);

    if (nd < 1){
        fprintf(stderr, "no split point\n");
        goto free_malloc;
    }

    int * ol = (int*)d;
    memset(ol, 0, sizeof(int) * n);

    for (int i = 0; i <= nd; i++){
        int b = 0, e = n, c = 0;
        if (i > 0) b = (int)sp[i - 1] + 1;
        if (i < nd) e = (int)sp[i] + 1;

        for (int j = b; j < e; j++) {
            c += l[j];
        }

        if (c > e - b - c){
            for (int j = b; j < e; j++){
                ol[j] = 1;
            }
        }
    }

    free(sp); sp = NULL;
    
    for (int i = 0; i < n; i++){
        printf("%d", ol[i]);
    }
    printf("\n");
    
free_malloc:
    free(LogD); LogD = NULL;
    free(d);  d = NULL;
    free(l);  l = NULL;

    return 0;
}
