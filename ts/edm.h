/* ========================================================
 *   Copyright (C) 2015 All rights reserved.
 *   
 *   filename : edm.h
 *   author   : liuzhiqiang01
 *   date     : 2015-01-04
 *   info     : find breakout based on median
 * ======================================================== */

#ifndef _EDM_H
#define _EDM_H

/* -----------------------------------------------------------------
 * @brief    : find breakout point using dynamic programing process
 *             based on median
 * @ts       : time series 
 * @n        : length of time series
 * @min_size : min size between two break out points
 * @beta     : penalty factor
 * @degree   : penalty degree 0:const, 1: linear, 2:quadratic
 * @ol       : number of break out outer param
 * @return   : break out index array
 * ----------------------------------------------------------------- */
int * breakout(double * ts, int n, int min_size, double percent, int degree, int *ol);

#endif //EDM_H

