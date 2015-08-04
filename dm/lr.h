/* ========================================================
 *   Copyright (C) 2015 All rights reserved.
 *
 *   filename : lr.h
 *   author   : ***
 *   date     : 2015-07-31
 *   info     : implementation of Logistic Regression 
 *   		using newton method
 * ======================================================== */

#ifndef _LR_H
#define _LR_H

/* -----------------------------------------
 * brief  : LR function 
 * r      : r instance
 * c      : c coefficient
 * tlen   : number of data
 * len    : length of each instance
 * id     : features ids
 * val    : features values
 * y      : label of each instance
 * lambda : regulized parameter
 * method : 1: L1, 2: L2
 * x      : return coefficient
 * ----------------------------------------- */
int lr(int r, int c, int tlen, int *len, int *id, double *val, double *y, double lambda, int method double *x);

#endif //LR
