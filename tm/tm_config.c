/* ========================================================
 *   Copyright (C) 2016 All rights reserved.
 *   
 *   filename : tm_config.c
 *   author   : liuzhiqiangruc@126.com
 *   date     : 2016-07-15
 *   info     : 
 * ======================================================== */
#include <stdlib.h>
#include "tm_config.h"

static void set(void * tmc, int argc, char * argv[]){
    ((TMConfig*)tmc)->a = 0.1;
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
    tmc->set = set;
    tmc->get_alpha = get_alpha;
    tmc->get_beta  = get_beta;
    tmc->get_n     = get_n;
    tmc->get_s     = get_s;
    tmc->get_k     = get_k;
    tmc->get_d     = get_d;
    tmc->get_o     = get_o;
    return tmc;
}

void free_config(TMConfig * tmc){
    free(tmc);
}
