/* ========================================================
 *   Copyright (C) 2015 All rights reserved.
 *   
 *   filename : viterbi.c
 *   author   : ***
 *   date     : 2015-04-29
 *   info     : 
 * ======================================================== */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "viterbi.h"


void help(){
    fprintf(stderr, "\nviterbi sequence regularization usage: \n");
    fprintf(stderr, "./viterbi -f <string> -n <int> -a <int> -b <int>\n\n");
    fprintf(stderr, "        -f input seq file one value per line \n");
    fprintf(stderr, "        -n line num of input seq file\n");
    fprintf(stderr, "        -a state transfer rate\n");
    fprintf(stderr, "        -b state --> obvisous rate\n");
}

int main(int argc, char * argv[]){
    if (argc < 5){
        help();
        return -1;
    }
    char * input = NULL;
    char * arg = NULL;
    int n = 0, a = 10, b = 5;
    int i = 0;
    while (i < argc){
        arg = argv[i];
        if (0 == strcmp(arg, "-f")){
            input = argv[++i];
        }
        else if (0 == strcmp(arg, "-n")){
            n = atoi(argv[++i]);
        }
        else if (0 == strcmp(arg, "-a")){
            a = atoi(argv[++i]);
        }
        else if (0 == strcmp(arg, "-b")){
            b = atoi(argv[++i]);
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
    int *ol = (int*)malloc(sizeof(int) * n);
    memset(l,  0, sizeof(int) * n);
    memset(ol, 0, sizeof(int) * n);
    for (int i = 0; i < n; i++){
        fscanf(f, "%d", l + i);
    }
    fclose(f);

    viterbi(l, ol, n, a, b);
    viterbi(l, ol, n, a, b);
    viterbi(l, ol, n, a, b);
    viterbi(l, ol, n, a, b);
    viterbi(l, ol, n, a, b);
    viterbi(l, ol, n, a, b);
    viterbi(l, ol, n, a, b);

    free(l);  l = NULL;

    for (int i = 0; i < n; i++){
        printf("%d\n", ol[i]);
    }

    free(ol);  ol = NULL;

    return 0;
}
