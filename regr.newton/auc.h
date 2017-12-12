/* ========================================================
*   Copyright (C) 2014 All rights reserved.
*
*   filename : auc.h
*   author   : liuzhiqiangruc@126.com
*   date     : 2015-7-28
*   info     : area user roc curve(metrics)
* ======================================================== */

#ifndef _AUC_H
#define _AUC_H
/*
 * x score
 * y pridict
 */
double auc(int n, double *x, double *y);
#endif //AUC_H
