/* ========================================================
 *   Copyright (C) 2015 All rights reserved.
 *
 *   filename : lr.h
 *   author   : ***
 *   date     : 2015-07-31
 *   info     : implementation of Logistic Regression using newton method
 * ======================================================== */

#ifndef _LR_H
#define _LR_H
/*
 * r:    r instance
 * c:    c coefficient
 * tlen: number of data
 * len:  length of each instance
 * id:
 * val:  <id , val> :  <key, value>
 * y:    label of each instance
 * x:    return coefficient
 */
int lr(int r, int c, int tlen, int *len, int *id, double *val, double *y, double *x);

#endif //LR