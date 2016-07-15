/* ========================================================
 *   Copyright (C) 2016 All rights reserved.
 *   
 *   filename : tm_config_h.h
 *   author   : liuzhiqiangruc@126.com
 *   date     : 2016-07-15
 *   info     : 
 * ======================================================== */

#ifndef _TM_CONFIG_H_H
#define _TM_CONFIG_H_H

#include "tm_config.h"

typedef struct _tmconfigh TMHConfig;

typedef double (*GET_G0)(void *);
typedef double (*GET_G1)(void *);

struct _tmconfigh{
    double g0;
    double g1;
    TMConfig * tmc;
    GET_ALPHA  get_alpha;
    GET_BETA   get_beta;
    GET_K      get_k;
    GET_N      get_n;
    GET_S      get_s;
    GET_D      get_d;
    GET_O      get_o;
    GET_G0     get_g0;
    GET_G1     get_g1;
    SET_CONFIG set;
};


TMHConfig * init_h_config();
void free_h_config(TMHConfig * tmc);

#endif //TM_CONFIG_H_H
