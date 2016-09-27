/* ========================================================
 *   Copyright (C) 2016 All rights reserved.
 *   
 *   filename : w2v_config.c
 *   author   : liuzhiqiangruc@126.com
 *   date     : 2016-09-22
 *   info     : 
 * ======================================================== */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "w2v_config.h"

static void help(void){
    fprintf(stderr, "Command Line Usage: \n");
    fprintf(stderr, "w2v -a [double] -k [int] -w [int] -d [string] -o [string]\n");
    fprintf(stderr, "    -a learning rate\n");
    fprintf(stderr, "    -k vector length\n");
    fprintf(stderr, "    -w window size  \n");
    fprintf(stderr, "    -d input data   \n");
    fprintf(stderr, "    -o out    dir   \n");
}

static int set(void * wc, int argc, char * argv[]){
    int k, w, i;
    double a;
    char *d, *o, *arg;
    k = 20, w = 5;
    a = 0.025;
    d = NULL, o = ".";
    if ((argc & 1) == 0){
        return -1;
    }
    i = 0, arg = NULL;
    while (i < argc){
        arg = argv[i];
        if (0 == strcmp(arg, "-a")){
            a = atof(argv[++i]);
        }
        else if (0 == strcmp(arg, "-k")){
            k = atoi(argv[++i]);
        }
        else if (0 == strcmp(arg, "-w")){
            w = atoi(argv[++i]);
        }
        else if (0 == strcmp(arg, "-d")){
            d = argv[++i];
        }
        else if (0 == strcmp(arg, "-o")){
            o = argv[++i];
        }
        i += 1;
    }
    if (NULL == d){
        return -2;
    }
    if (w < 2){
        fprintf(stderr, "w can not be less than 2\n");
    }
    ((W2VConfig *)wc)->a = a;
    ((W2VConfig *)wc)->k = k;
    ((W2VConfig *)wc)->w = w;
    ((W2VConfig *)wc)->d = d;
    ((W2VConfig *)wc)->o = o;
    return 0;
}

static void free_conf(void *wc){
    free(wc);
}

static double get_alpha(void *wc){
    return ((W2VConfig*)wc)->a;
}

static int get_k(void *wc){
    return ((W2VConfig*)wc)->k;
}

static int get_w(void *wc){
    return ((W2VConfig*)wc)->w;
}

static char * get_d(void *wc){
    return ((W2VConfig*)wc)->d;
}

static char * get_o(void *wc){
    return ((W2VConfig*)wc)->o;
}

W2VConfig * init_config(){
    W2VConfig * wc = (W2VConfig*)calloc(1, sizeof(W2VConfig));
    wc->set        = set;
    wc->help       = help;
    wc->free       = free_conf;
    wc->get_alpha  = get_alpha;
    wc->get_k      = get_k;
    wc->get_w      = get_w;
    wc->get_d      = get_d;
    wc->get_o      = get_o;
    return wc;
}

