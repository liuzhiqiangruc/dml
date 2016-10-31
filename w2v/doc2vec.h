/* ========================================================
 *   Copyright (C) 2016 All rights reserved.
 *   
 *   filename : doc2vec.h
 *   author   : liuzhiqiangruc@126.com
 *   date     : 2016-10-27
 *   info     : 
 * ======================================================== */

#ifndef _DOC2VEC_H
#define _DOC2VEC_H


#include "vec.h"
#include "d2v_config.h"

typedef struct _dw{
    TSD * ds;
    Vec * model;
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


#endif //DOC2VEC_H
