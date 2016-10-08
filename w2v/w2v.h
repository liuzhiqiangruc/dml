/* ========================================================
 *   Copyright (C) 2016 All rights reserved.
 *   
 *   filename : w2v.h
 *   author   : liuzhiqiangruc@126.com
 *   date     : 2016-09-22
 *   info     : word2vec implementation of cbow using hsmax
 * ======================================================== */

#ifndef _W2V_H
#define _W2V_H

#define KEY_SIZE 64
#define LINE_LEN 1024

typedef struct _w2v WV;

WV * wv_create(int argc, char * argv[]);
int  wv_init(WV * wv);
void wv_est (WV * wv);
void wv_save(WV * wv);
void wv_free(WV * wv);
int  wv_dsize(WV * wv);
int  wv_vsize(WV * wv);
int  wv_tsize(WV * wv);

#endif //W2V_H
