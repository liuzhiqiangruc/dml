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
    fprintf(stderr, "w2v -a [double] -k [int] -n [int] -m [int] -t [0|1|2] -w [int] -d [string] -o [string]\n");
    fprintf(stderr, "    -a learning rate\n");
    fprintf(stderr, "    -k vector length\n");
    fprintf(stderr, "    -n iter number  \n");
    fprintf(stderr, "    -m thread num   \n");
    fprintf(stderr, "    -t 0:learn, 1:continue learn, 2: hidden fix\n");
    fprintf(stderr, "    -w window size  \n");
    fprintf(stderr, "    -d input data   \n");
    fprintf(stderr, "    -o out    dir   \n");
}

static int set(void * wc, int argc, char * argv[]){
    int k, w, i, t, n, m;
    double a;
    char *d, *o, *arg;
    k = 20, w = 5, t = 0, n = 5, m = 1;
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
        else if (0 == strcmp(arg, "-t")){
            t = atoi(argv[++i]);
        }
        else if (0 == strcmp(arg, "-n")){
            n = atoi(argv[++i]);
        }
        else if (0 == strcmp(arg, "-m")){
            m = atoi(argv[++i]);
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
    if (t < 2 && NULL == d){
        return -2;
    }
    if (w < 2){
        fprintf(stderr, "w can not be less than 2\n");
    }
    ((W2VConfig *)wc)->a = a;
    ((W2VConfig *)wc)->k = k;
    ((W2VConfig *)wc)->t = t;
    ((W2VConfig *)wc)->n = n;
    ((W2VConfig *)wc)->m = m;
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

static int get_t(void *wc){
    return ((W2VConfig*)wc)->t;
}

static int get_k(void *wc){
    return ((W2VConfig*)wc)->k;
}

static int get_n(void *wc){
    return ((W2VConfig*)wc)->n;
}

static int get_m(void *wc){
    return ((W2VConfig*)wc)->m;
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
    wc->get_n      = get_n;
    wc->get_m      = get_m;
    wc->get_t      = get_t;
    wc->get_w      = get_w;
    wc->get_d      = get_d;
    wc->get_o      = get_o;
    return wc;
}

W2VConfig * init_w2v_config(){
    return init_config();
}
