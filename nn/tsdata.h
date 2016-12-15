/* ========================================================
 *   Copyright (C) 2016 All rights reserved.
 *   
 *   filename : tdata.h
 *   author   : liuzhiqiangruc@126.com
 *   date     : 2016-10-27
 *   info     : data wrap for transaction data
 * ======================================================== */

#ifndef _TDDAT_H
#define _TDDAT_H

#include "hash.h"

#define KEY_SIZE 128

// transaction type define
typedef struct _tdata {
    int   d;
    int   v;
    int   t;
    int   * doffs;
    int   * tokens;
    int  (*fcnt)[2];
    char (*idm)[KEY_SIZE];
} TSD;

TSD * tsd_load(char * infile);

TSD * tsd_load_v(char * infile, Hash * vhs);

void tsd_free(TSD * ds);

#endif //TDDAT_H
