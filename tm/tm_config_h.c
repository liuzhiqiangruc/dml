/* ========================================================
 *   Copyright (C) 2016 All rights reserved.
 *   
 *   filename : tm_config_h.c
 *   author   : liuzhiqiangruc@126.com
 *   date     : 2016-07-15
 *   info     : 
 * ======================================================== */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tm_config_h.h"

static void help(void){
    fprintf(stderr, "Command Line Usage:\n");
    fprintf(stderr, "./hlda -a [double] -b [double] -g0 [double] -g1 [double] -k [int] -n [int] -s [int] -d [string] -o [string]\n");
    fprintf(stderr, "      -a   doc_topic prior param \n");
    fprintf(stderr, "      -b   topic_word prior param\n");
    fprintf(stderr, "      -g0  global prior param    \n");
    fprintf(stderr, "      -g1  special prior param   \n");
    fprintf(stderr, "      -k   topic count           \n");
    fprintf(stderr, "      -n   iter number           \n");
    fprintf(stderr, "      -s   save step             \n");
    fprintf(stderr, "      -d   input file            \n");
    fprintf(stderr, "      -o   out dir               \n");
}

static int set(void * tmc, int argc, char * argv[]){
    double g0, g1;
    int i, r;
    char * arg;
    r = ((TMHConfig*)tmc)->tmc->set(((TMHConfig*)tmc)->tmc, argc, argv);
    if (r != 0){
        return r;
    }
    g0 = g1 = 0.5;
    i = 0, arg = NULL;
    while (i < argc){
        arg = argv[i];
        if (0 == strcmp(arg, "-g0")){
            g0 = atof(argv[++i]);
        }
        else if (0 == strcmp(arg, "-g1")){
            g1 = atof(argv[++i]);
        }
        i += 1;
    }
    ((TMHConfig*)tmc)->g0 = g0;
    ((TMHConfig*)tmc)->g1 = g1;
    return 0;
}

static void free_conf(void *tmc){
    TMHConfig * t = ((TMHConfig*)tmc);
    t->tmc->free(t->tmc);
    t->tmc = NULL;
    free(tmc);
}

static double get_alpha(void * tmc){
    TMHConfig * t = ((TMHConfig*)tmc);
    return t->tmc->get_alpha(t->tmc);
}

static double get_beta(void * tmc){
    TMHConfig * t = ((TMHConfig*)tmc);
    return t->tmc->get_beta(t->tmc);
}

static int get_k(void *tmc){
    TMHConfig * t = ((TMHConfig*)tmc);
    return t->tmc->get_k(t->tmc);
}

static int get_n(void *tmc){
    TMHConfig * t = ((TMHConfig*)tmc);
    return t->tmc->get_n(t->tmc);
}

static int get_s(void *tmc){
    TMHConfig * t = ((TMHConfig*)tmc);
    return t->tmc->get_s(t->tmc);
}

static char * get_d(void *tmc){
    TMHConfig * t = ((TMHConfig*)tmc);
    return t->tmc->get_d(t->tmc);
}

static char * get_o(void *tmc){
    TMHConfig * t = ((TMHConfig*)tmc);
    return t->tmc->get_o(t->tmc);
}

static double get_g0(void *tmc){
    return ((TMHConfig*)tmc)->g0;
}

static double get_g1(void *tmc){
    return ((TMHConfig*)tmc)->g1;
}

TMHConfig * init_h_config(){
    TMHConfig * tmc = (TMHConfig*)calloc(1, sizeof(TMHConfig));
    tmc->tmc        = init_config();
    tmc->set        = set;
    tmc->help       = help;
    tmc->free       = free_conf;
    tmc->get_alpha  = get_alpha;
    tmc->get_beta   = get_beta;
    tmc->get_n      = get_n;
    tmc->get_s      = get_s;
    tmc->get_k      = get_k;
    tmc->get_d      = get_d;
    tmc->get_o      = get_o;
    tmc->get_g0     = get_g0;
    tmc->get_g1     = get_g1;
    return tmc;
}
