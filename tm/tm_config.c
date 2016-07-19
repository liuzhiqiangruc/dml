/* ========================================================
 *   Copyright (C) 2016 All rights reserved.
 *   
 *   filename : tm_config.c
 *   author   : liuzhiqiangruc@126.com
 *   date     : 2016-07-15
 *   info     : 
 * ======================================================== */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tm_config.h"

static void help(void){
    fprintf(stderr, "Command Line Usage:\n");
    fprintf(stderr, "./lda -a [double] -b [double] -k [int] -n [int] -s [int] -d [string] -o [string]\n");
    fprintf(stderr, "      -a   doc_topic prior param \n");
    fprintf(stderr, "      -b   topic_word prior param\n");
    fprintf(stderr, "      -k   topic count           \n");
    fprintf(stderr, "      -n   iter number           \n");
    fprintf(stderr, "      -s   save step             \n");
    fprintf(stderr, "      -d   input file            \n");
    fprintf(stderr, "      -o   out dir               \n");
}

static int set(void * tmc, int argc, char * argv[]){
    int k, n, s, i;
    double a, b;
    char *d, *o, *arg;
    k = 50, n = 20, s = 10;
    a = 0.05, b = 0.01;
    d = NULL, o = ".";
    if ((argc & 1) == 0){
        return -1;
    }
    i = 0, arg = NULL;
    while (i < argc) {
        arg = argv[i];
        if (0 == strcmp(arg, "-a")){
            a = atof(argv[++i]);
        }
        else if (0 == strcmp(arg, "-b")){
            b = atof(argv[++i]);
        }
        else if (0 == strcmp(arg, "-k")){
            k = atoi(argv[++i]);
        }
        else if (0 == strcmp(arg, "-n")){
            n = atoi(argv[++i]);
        }
        else if (0 == strcmp(arg, "-s")){
            s = atoi(argv[++i]);
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
    ((TMConfig*)tmc)->a = a;
    ((TMConfig*)tmc)->b = b;
    ((TMConfig*)tmc)->k = k;
    ((TMConfig*)tmc)->n = n;
    ((TMConfig*)tmc)->s = s;
    ((TMConfig*)tmc)->d = d;
    ((TMConfig*)tmc)->o = o;
    return 0;
}

static void free_conf(void *tmc){
    free(tmc);
}

static double get_alpha(void * tmc){
    return ((TMConfig*)tmc)->a;
}

static double get_beta(void * tmc){
    return ((TMConfig*)tmc)->b;
}

static int get_k(void *tmc){
    return ((TMConfig*)tmc)->k;
}

static int get_n(void *tmc){
    return ((TMConfig*)tmc)->n;
}

static int get_s(void *tmc){
    return ((TMConfig*)tmc)->s;
}

static char * get_d(void *tmc){
    return ((TMConfig*)tmc)->d;
}

static char * get_o(void *tmc){
    return ((TMConfig*)tmc)->o;
}

TMConfig * init_config(){
    TMConfig * tmc = (TMConfig*)calloc(1, sizeof(TMConfig));
    tmc->set       = set;
    tmc->help      = help;
    tmc->free      = free_conf;
    tmc->get_alpha = get_alpha;
    tmc->get_beta  = get_beta;
    tmc->get_n     = get_n;
    tmc->get_s     = get_s;
    tmc->get_k     = get_k;
    tmc->get_d     = get_d;
    tmc->get_o     = get_o;
    return tmc;
}
