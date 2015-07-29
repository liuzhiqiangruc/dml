/* ========================================================
 *   Copyright (C) 2015 All rights reserved.
 *
 *   filename : fitl2.h
 *   author   : lizeming@baidu.com
 *   		liuzhiqiang01@baidu.com
 *   date     : 2015-04-14
 *   info     : time series fit with L1, L2 Norm
 * ======================================================== */


#ifndef _FITL2_NEWTON_H
#define _FITL2_NEWTON_H

void fit_newton(int n, double *datax, double l2, int method, int m, int it, double *retx);

#endif
