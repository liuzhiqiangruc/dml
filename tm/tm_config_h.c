/* ========================================================
 *   Copyright (C) 2016 All rights reserved.
 *   
 *   filename : tm_config_h.c
 *   author   : liuzhiqiangruc@126.com
 *   date     : 2016-07-15
 *   info     : 
 * ======================================================== */
#include <stdlib.h>
#include "tm_config_h.h"

static void set(void * tmc, int argc, char * argv[]){
    ((TMHConfig*)tmc)->g0 = 0.1;
    ((TMHConfig*)tmc)->tmc->set(((TMHConfig*)tmc)->tmc, argc, argv);
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

TMHConfig * init_h_config(){
    TMHConfig * tmc = (TMHConfig*)calloc(1, sizeof(TMHConfig));
    tmc->tmc        = init_config();
    tmc->set        = set;
    tmc->free       = free_conf;
    tmc->get_alpha  = get_alpha;
    tmc->get_beta   = get_beta;
    tmc->get_n      = get_n;
    tmc->get_s      = get_s;
    tmc->get_k      = get_k;
    tmc->get_d      = get_d;
    tmc->get_o      = get_o;
    return tmc;
}
