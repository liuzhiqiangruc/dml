/* ========================================================
 *   Copyright (C) 2016 All rights reserved.
 *   
 *   filename : tm_config_x.c
 *   author   : liuzhiqiangruc@126.com
 *   date     : 2016-07-19
 *   info     : 
 * ======================================================== */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tm_config_x.h"

static void help(void){
    fprintf(stderr, "Command Line Usage:\n");
    fprintf(stderr, "./xlda -a [double] -b [double] -g0 [double] -g1 [double] -u [double] -k [int] -l [int] -n [int] -s [int] -d [string] -o [string]\n");
    fprintf(stderr, "      -a   doc_topic prior param \n");
    fprintf(stderr, "      -b   topic_word prior param\n");
    fprintf(stderr, "      -g0  global prior param    \n");
    fprintf(stderr, "      -g1  special prior param   \n");
    fprintf(stderr, "      -u   special D param       \n");
    fprintf(stderr, "      -k   topic count           \n");
    fprintf(stderr, "      -l   special cls count     \n");
    fprintf(stderr, "      -n   iter number           \n");
    fprintf(stderr, "      -s   save step             \n");
    fprintf(stderr, "      -d   input file            \n");
    fprintf(stderr, "      -o   out dir               \n");
}

static int set(void *tmc, int argc, char * argv[]){
    int l, i;
    double u, r;
    char * arg;
    r = ((TMXConfig*)tmc)->tmc->set(((TMXConfig*)tmc)->tmc, argc, argv);
    if (r != 0){
        return r;
    }
    l = 30,  u = 1.0;
    i = 0, arg = NULL;
    while (i < argc){
        arg = argv[i];
        if (0 == strcmp(arg, "-u")){
            u = atof(argv[++i]);
        }
        else if (0 == strcmp(arg, "-l")){
            l = atof(argv[++i]);
        }
        i += 1;
    }
    ((TMXConfig*)tmc)->l = l;
    ((TMXConfig*)tmc)->u = u;
    return 0;
}

static void free_conf(void *tmc){
    TMXConfig * t = ((TMXConfig*)tmc);
    t->tmc->free(t->tmc);
    t->tmc = NULL;
    free(tmc);
}

static double get_alpha(void *tmc){
    TMXConfig * t = (TMXConfig*)tmc;
    return t->tmc->get_alpha(t->tmc);
}

static double get_beta(void *tmc){
    TMXConfig * t = (TMXConfig*)tmc;
    return t->tmc->get_beta(t->tmc);
}

static int get_k(void *tmc){
    TMXConfig * t = ((TMXConfig*)tmc);
    return t->tmc->get_k(t->tmc);
}

static int get_n(void *tmc){
    TMXConfig * t = ((TMXConfig*)tmc);
    return t->tmc->get_n(t->tmc);
}

static int get_s(void *tmc){
    TMXConfig * t = ((TMXConfig*)tmc);
    return t->tmc->get_s(t->tmc);
}

static char * get_d(void *tmc){
    TMXConfig * t = ((TMXConfig*)tmc);
    return t->tmc->get_d(t->tmc);
}

static char * get_o(void *tmc){
    TMXConfig * t = ((TMXConfig*)tmc);
    return t->tmc->get_o(t->tmc);
}

static double get_g0(void *tmc){
    TMXConfig * t = ((TMXConfig*)tmc);
    return t->tmc->get_g0(t->tmc);
}

static double get_g1(void *tmc){
    TMXConfig * t = ((TMXConfig*)tmc);
    return t->tmc->get_g1(t->tmc);
}

static double get_u(void *tmc){
    return ((TMXConfig*)tmc)->u;
}

static int    get_l(void *tmc){
    return ((TMXConfig*)tmc)->l;
}

TMXConfig * init_x_config(){
    TMXConfig * tmc = (TMXConfig*)calloc(1, sizeof(TMXConfig));
    tmc->tmc        = init_h_config();
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
    tmc->get_u      = get_u;
    tmc->get_l      = get_l;
    return tmc;
}
