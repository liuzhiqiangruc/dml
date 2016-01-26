/* ========================================================
 *   Copyright (C) 2015 All rights reserved.
 *
 *   filename : loess.h
 *   author   : lizeming@baidu.com
 *              liuzhiqiang@baidu.com
 *   date     : 2015-04-17
 *   info     : local weighted 2-d regression
 * ======================================================== */



#ifndef _LOESS_H
#define _LOESS_H

/* ------------------------------------------------
 * brief : local weighted regression for timeseries
 *         with 2-degree
 * datax : input data
 * n     : length of input
 * maxDist : local span length
 * robust  : count of robust operation 
 * rety    : regressed time series smoothed
 * ------------------------------------------------ */
void loess(double *datax, int n, int span, int robust, double *rety);

#endif
