/* ========================================================
 *   Copyright (C) 2014 All rights reserved.
 *   
 *   filename : kmeans.h
 *   author   : liuzhiqiang01@baidu.com
 *   date     : 2014-09-19
 *   info     : implementation of kmeans++
 * ======================================================== */

#ifndef _KMEANS_H
#define _KMEANS_H

/* ------------------------------------------------
 * brief     : kmeans++
 * float * m : input data
 * int     n : number of rows   of input
 * int     f : number of column of input
 * int     k : cluster number
 * int   * c : cluster ID from 0 ~ k-1
 * return    : 0 success else failed
 * ------------------------------------------------ */
int kmeans(float * m, int n, int f, int k, int * c);


#endif //KMEANS_H
