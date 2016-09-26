/* ========================================================
 *   Copyright (C) 2016 All rights reserved.
 *   
 *   filename : w2v_config.h
 *   author   : liuzhiqiangruc@126.com
 *   date     : 2016-09-22
 *   info     : 
 * ======================================================== */

#ifndef _W2V_CONFIG_H
#define _W2V_CONFIG_H

typedef void   (*HELP)         (void);
typedef int    (*SET_CONFIG)   (void * wc, int argc, char *argv[]);
typedef void   (*FREE_CONF)    (void * wc);
typedef double (*GET_ALPHA)    (void * wc);
typedef int    (*GET_K)        (void * wc);
typedef int    (*GET_W)        (void * wc);
typedef char * (*GET_D)        (void * wc);
typedef char * (*GET_O)        (void * wc);


typedef struct _w2vconfig {
    double a;
    int    k;
    int    w;
    char * d;
    char * o;
    GET_ALPHA  get_alpha;
    GET_K      get_k;
    GET_W      get_w;
    GET_D      get_d;
    GET_O      get_o;
    SET_CONFIG set;
    HELP       help;
    FREE_CONF  free;
} W2VConfig;

W2VConfig * init_config();

#endif //W2V_CONFIG_H
