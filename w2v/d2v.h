/* ========================================================
 *   Copyright (C) 2016 All rights reserved.
 *   
 *   filename : d2v.h
 *   author   : liuzhiqiangruc@126.com
 *   date     : 2016-10-21
 *   info     : 
 * ======================================================== */

#ifndef _D2V_H
#define _D2V_H

typedef struct _d2v DV;

DV * dv_create(int argc, char * argv[]);
int  dv_init (DV * dv);
void dv_est  (DV * dv);
void dv_save (DV * dv);
void dv_free (DV * dv);

int dv_dsize(DV * dv);
int dv_vsize(DV * dv);
int dv_tsize(DV * dv);


#endif //D2V_H
