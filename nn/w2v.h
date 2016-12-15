/* ========================================================
 *   Copyright (C) 2016 All rights reserved.
 *   
 *   filename : w2v.h
 *   author   : ***
 *   date     : 2016-12-14
 *   info     : 
 * ======================================================== */

#ifndef _W2V_H
#define _W2V_H

#include "hsoft.h"
#include "tsdata.h"
#include "w2v_config.h"

typedef struct _w2v_s{
    TSD * ds;
    HSoft * hsf;
    double * u;
    W2VConfig * wc;
} W2V;

W2V * w2v_create(int argc, char *argv[]);
int   w2v_init  (W2V * w2v);
void  w2v_learn (W2V * w2v);
void  w2v_save  (W2V * w2v);
void  w2v_free  (W2V * w2v);

int   w2v_dsize (W2V * w2v);
int   w2v_vsize (W2V * w2v);
int   w2v_tsize (W2V * w2v);

#endif //W2V_H
