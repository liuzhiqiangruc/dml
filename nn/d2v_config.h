/* ========================================================
 *   Copyright (C) 2016 All rights reserved.
 *   
 *   filename : d2v_config.h
 *   author   : liuzhiqiangruc@126.com
 *   date     : 2016-10-21
 *   info     : 
 * ======================================================== */

#ifndef _D2V_CONFIG_H
#define _D2V_CONFIG_H

#include "w2v_config.h"

typedef struct _d2v_config D2VConfig;

struct _d2v_config{
    W2VConfig * cfg;
    GET_ALPHA  get_alpha;
    GET_K      get_k;
    GET_T      get_t;
    GET_N      get_n;
    GET_D      get_d;
    GET_O      get_o;
    SET_CONFIG set;
    HELP       help;
    FREE_CONF  free;
};

D2VConfig * init_d_config();


#endif //D2V_CONFIG_H
