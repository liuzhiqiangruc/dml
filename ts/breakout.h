/* ========================================================
 *   Copyright (C) 2016 All rights reserved.
 *   
 *   filename : breakout.h
 *   author   : ***
 *   date     : 2016-01-15
 *   info     : 
 * ======================================================== */

#ifndef _BREAKOUT_H
#define _BREAKOUT_H

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
int * edm(double * ts, int n, int min_size, double percent, int degree, int *ol);

/* -----------------------------------------------------
 * @brief    : find breakout point using dynamic programing process
 *             based on linear regression
 * @ts       : time series 
 * @n        : length of ts
 * @min_size : min size of sub ts 
 * @beta     : regularized param for number of split point
 * @ol       : number of split point
 * @rety     : regenerated ts if not null
 * @return   : array of split point index
 * ----------------------------------------------------- */
int * lsp(double * ts, int n, int min_size, double beta, int *ol, double * rety);

/* --------------------------------------------------------------
 * @brief    : bayesian changepoint detection for time series
 * @data     : time series
 * @T        : length of ts
 * points    : check value for each point which is cp or not
 * -------------------------------------------------------------- */
void bcp(double *data, int T,int *points);

#endif //BREAKOUT_H
