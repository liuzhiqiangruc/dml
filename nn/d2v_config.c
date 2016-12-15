/* ========================================================
 *   Copyright (C) 2016 All rights reserved.
 *   
 *   filename : d2v_config.c
 *   author   : liuzhiqiangruc@126.com
 *   date     : 2016-10-21
 *   info     : there is no "w" configration for doc2vec
 * ======================================================== */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "d2v_config.h"

static void help(void){
    fprintf(stderr, "Command Line Usage: \n");
    fprintf(stderr, "d2v -a [double] -k [int] -n [int] -t [0|1|2] -d [string] -o [string]\n");
    fprintf(stderr, "    -a learning rate\n");
    fprintf(stderr, "    -k vector length\n");
    fprintf(stderr, "    -n iter number  \n");
    fprintf(stderr, "    -t 0:learn, 1:continue learn, 2: continue learn while huftree not change\n");
    fprintf(stderr, "    -d input data   \n");
    fprintf(stderr, "    -o out    dir   \n");
}

static int set(void * dc, int argc, char * argv[]){
    return ((D2VConfig*)dc)->cfg->set(((D2VConfig*)dc)->cfg, argc, argv);
}

static void free_conf(void *dc){
    D2VConfig * t = ((D2VConfig*)dc);
    t->cfg->free(t->cfg);
    t->cfg = NULL;
    free(dc);
}

static double get_alpha(void *dc){
    D2VConfig * t = ((D2VConfig*)dc);
    return t->cfg->get_alpha(t->cfg);
}

static int get_t(void *dc){
    D2VConfig * t = ((D2VConfig*)dc);
    return t->cfg->get_t(t->cfg);
}

static int get_k(void *dc){
    D2VConfig * t = ((D2VConfig*)dc);
    return t->cfg->get_k(t->cfg);
}

static int get_n(void *dc){
    D2VConfig * t = ((D2VConfig*)dc);
    return t->cfg->get_n(t->cfg);
}

static char * get_d(void *dc){
    D2VConfig * t = ((D2VConfig*)dc);
    return t->cfg->get_d(t->cfg);
}

static char * get_o(void *dc){
    D2VConfig * t = ((D2VConfig*)dc);
    return t->cfg->get_o(t->cfg);
}

D2VConfig * init_d_config(){
    D2VConfig * dc = (D2VConfig*)calloc(1, sizeof(D2VConfig));
    dc->cfg        = init_config();
    dc->set        = set;
    dc->help       = help;
    dc->free       = free_conf;
    dc->get_alpha  = get_alpha;
    dc->get_k      = get_k;
    dc->get_n      = get_n;
    dc->get_t      = get_t;
    dc->get_d      = get_d;
    dc->get_o      = get_o;
    return dc;
}
