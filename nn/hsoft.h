/* ========================================================
 *   Copyright (C) 2016 All rights reserved.
 *   
 *   filename : hsoft.h
 *   author   : liuzhiqiangruc@126.com
 *   date     : 2016-12-06
 *   info     : hierarchical softmax implementation
 * ======================================================== */

#ifndef _HSOFT_H
#define _HSOFT_H

// hierarchal softmax struct
typedef struct _hsoft {
    int v;         // voc size
    int k;         // dim of latent space
    int (*hbt)[5]; // huffman tree {p0, b0, p1, b1, index}
    double * tree; // noleaf nodes weights, totle:v - 1
} HSoft;

// wc : word cnt 
// n  : word size, voc size
void hsoft_build(HSoft ** hsf, int (*wc)[2], int v, int k);

// in    : input vector
// out   : gradient vector
// tid   : target word id
// alpha : learning rate
double hsoft_learn(HSoft * hsf, double *in, double *out, int tid, double alpha);

// free
void hsoft_free (HSoft * hsf);

// save
void hsoft_save (HSoft * hsf, const char * outdir);

// load
int  hsoft_load (HSoft ** hsf, const char * outdir, int k);

#endif //HSOFT_H
