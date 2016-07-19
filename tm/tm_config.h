/* ========================================================
 *   Copyright (C) 2016 All rights reserved.
 *   
 *   filename : tm_config.h
 *   author   : liuzhiqiangruc@126.com
 *   date     : 2016-07-15
 *   info     : 
 * ======================================================== */

#ifndef _TM_CONFIG_H
#define _TM_CONFIG_H

typedef void   (*HELP)      (void);
typedef int    (*SET_CONFIG)(void * tmc, int argc, char * argv[]);
typedef void   (*FREE_CONF) (void * tmc);
typedef double (*GET_ALPHA) (void * tmc);
typedef double (*GET_BETA)  (void * tmc);
typedef int    (*GET_K)     (void * tmc);
typedef int    (*GET_N)     (void * tmc);
typedef int    (*GET_S)     (void * tmc);
typedef char * (*GET_D)     (void * tmc);
typedef char * (*GET_O)     (void * tmc);

typedef struct _tmconfig{
    double a;
    double b;
    int    k;
    int    n;
    int    s;
    char * d;
    char * o;
    GET_ALPHA  get_alpha;
    GET_BETA   get_beta;
    GET_K      get_k;
    GET_N      get_n;
    GET_S      get_s;
    GET_D      get_d;
    GET_O      get_o;
    SET_CONFIG set;
    HELP       help;
    FREE_CONF  free;
} TMConfig;


TMConfig * init_config();

#endif //TM_CONFIG_H
