/* ========================================================
 *   Copyright (C) 2016 All rights reserved.
 *   
 *   filename : tm.h
 *   author   : liuzhiqiangruc@126.com
 *   date     : 2016-07-15
 *   info     : 
 * ======================================================== */

#ifndef _TM_H
#define _TM_H

#define KEY_SIZE 64
#define LDA_LINE_LEN 1024

typedef struct _tm TM;

TM * tm_create(int argc, char *argv[]);
int  tm_init(TM * tm);
void tm_est (TM * tm);
void tm_save(TM * tm, int n);
void tm_free(TM * tm);

#endif //TM_H
