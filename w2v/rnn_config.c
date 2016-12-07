/* ========================================================
 *   Copyright (C) 2016 All rights reserved.
 *   
 *   filename : rnn_config.c
 *   author   : ***
 *   date     : 2016-12-06
 *   info     : 
 * ======================================================== */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "rnn_config.h"

static void help(void){
    fprintf(stderr, "Command line usage: \n");
    fprintf(stderr, "./rnnlm -a [double] -k [int] -w [int] -n [int] -d [string] -o [string]\n");
    fprintf(stderr, "        -a learning rate      \n");
    fprintf(stderr, "        -k latent space dim   \n");
    fprintf(stderr, "        -w bp depth for learn \n");
    fprintf(stderr, "        -n iteration number   \n");
    fprintf(stderr, "        -d input data         \n");
    fprintf(stderr, "        -d output dir         \n");
}

static int set(void *rc, int argc, char *argv[]){
    int k, w, n, i;
    double a;
    char *in, *out, *arg;

    k = 50, w = 3, n = 1;
    a = 0.001;
    in = NULL, out = ".";

    if ((argc & 1) == 0){
        return -1;
    }

    i = 0;
    arg = NULL; 

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
        else if (0 == strcmp(arg, "-n")){
            n = atoi(argv[++i]);
        }
        else if (0 == strcmp(arg, "-d")){
            in = argv[++i];
        }
        else if (0 == strcmp(arg, "-o")){
            out = argv[++i];
        }
        i += 1;
    }
    if (NULL == in){
        return -2;
    }
    ((RNNConfig*)rc)->a = a;
    ((RNNConfig*)rc)->k = k;
    ((RNNConfig*)rc)->w = w;
    ((RNNConfig*)rc)->n = n;
    ((RNNConfig*)rc)->d = in;
    ((RNNConfig*)rc)->o = out;

    return 0;
}

static void free_conf(void *rc){
    free(rc);
}

static double get_alpha(void *rc){
    return ((RNNConfig*)rc)->a;
}

static int get_k(void *rc){
    return ((RNNConfig*)rc)->k;
}

static int get_w(void *rc){
    return ((RNNConfig*)rc)->w;
}

static int get_n(void *rc){
    return ((RNNConfig*)rc)->n;
}

static char * get_d(void *rc){
    return ((RNNConfig*)rc)->d;
}

static char * get_o(void *rc){
    return ((RNNConfig*)rc)->o;
}

RNNConfig * init_rnn_config(){
    RNNConfig * rc = (RNNConfig*)calloc(1, sizeof(RNNConfig));
    rc->set        = set;
    rc->help       = help;
    rc->free       = free_conf;
    rc->get_alpha  = get_alpha;
    rc->get_k      = get_k;
    rc->get_w      = get_w;
    rc->get_n      = get_n;
    rc->get_d      = get_d;
    rc->get_o      = get_o;
    return rc;
}
