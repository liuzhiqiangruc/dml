/* ========================================================
 *   Copyright (C) 2016 All rights reserved.
 *   
 *   filename : tm_config_x.h
 *   author   : liuzhiqiangruc@126.com
 *   date     : 2016-07-19
 *   info     : 
 * ======================================================== */
#ifndef _TM_CONFIG_X_H
#define _TM_CONFIG_X_H

#include "tm_config_h.h"

typedef struct _tmconfigx TMXConfig;

typedef double (*GET_U)(void *);
typedef int    (*GET_L)(void *);

struct _tmconfigx{
    int l;
    double u;
    TMHConfig * tmc;
    GET_ALPHA  get_alpha;
    GET_BETA   get_beta;
    GET_K      get_k;
    GET_N      get_n;
    GET_S      get_s;
    GET_D      get_d;
    GET_O      get_o;
    GET_G0     get_g0;
    GET_G1     get_g1;
    GET_U      get_u;
    GET_L      get_l;
    SET_CONFIG set;
    HELP       help;
    FREE_CONF  free;
};

TMXConfig * init_x_config();

#endif //TM_CONFIG_X_H
