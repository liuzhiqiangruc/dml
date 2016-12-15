/* ========================================================
 *   Copyright (C) 2016 All rights reserved.
 *   
 *   filename : rnn_config.h
 *   author   : ***
 *   date     : 2016-12-06
 *   info     : 
 * ======================================================== */

#ifndef _RNN_CONFIG_H
#define _RNN_CONFIG_H

typedef void   (*HELP)         (void);
typedef int    (*SET_CONFIG)   (void * wc, int argc, char *argv[]);
typedef void   (*FREE_CONF)    (void * wc);
typedef double (*GET_ALPHA)    (void * wc);
typedef int    (*GET_K)        (void * wc);
typedef int    (*GET_W)        (void * wc);
typedef int    (*GET_T)        (void * wc);
typedef int    (*GET_N)        (void * wc);
typedef char * (*GET_D)        (void * wc);
typedef char * (*GET_O)        (void * wc);

typedef struct _rnn_config{
    double a;
    int    k;
    int    w;
    int    t;
    int    n;
    char * d;
    char * o;
    GET_ALPHA  get_alpha;
    GET_K      get_k;
    GET_W      get_w;
    GET_T      get_t;
    GET_N      get_n;
    GET_D      get_d;
    GET_O      get_o;
    SET_CONFIG set;
    HELP       help;
    FREE_CONF  free;
} RNNConfig;

RNNConfig * init_rnn_config();

#endif //RNN_CONFIG_H
