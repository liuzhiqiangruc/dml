/* ========================================================
 *   Copyright (C) 2016 All rights reserved.
 *   
 *   filename : d2v.h
 *   author   : ***
 *   date     : 2016-12-15
 *   info     : 
 * ======================================================== */

#ifndef _D2V_H
#define _D2V_H

#include "hsoft.h"
#include "d2v_config.h"
#include "tsdata.h"

typedef struct _d2v_s {
    TSD * ds;
    HSoft * hsf;
    double * u;
    int    * id;
    D2VConfig * dc;
} D2V;

D2V * d2v_create(int argc, char *argv[]);
int   d2v_init  (D2V * d2v);
void  d2v_learn (D2V * d2v);
void  d2v_save  (D2V * d2v);
void  d2v_free  (D2V * d2v);

int   d2v_dsize (D2V * d2v);
int   d2v_vsize (D2V * d2v);
int   d2v_tsize (D2V * d2v);

#endif //D2V_H
