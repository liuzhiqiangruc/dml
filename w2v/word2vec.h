/* ========================================================
 *   Copyright (C) 2016 All rights reserved.
 *   
 *   filename : word2vec.h
 *   author   : liuzhiqiangruc@126.com
 *   date     : 2016-10-27
 *   info     : 
 * ======================================================== */

#ifndef _WORD2VEC_H
#define _WORD2VEC_H

#include "vec.h"
#include "w2v_config.h"

typedef struct _wv{
    TSD * ds;
    Vec * model;
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

#endif //WORD2VEC_H
