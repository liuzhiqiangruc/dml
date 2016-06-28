/* ========================================================
 *   Copyright (C) 2016 All rights reserved.
 *   
 *   filename : k2d.h
 *   author   : liuzhiqiangruc@126.com
 *   date     : 2016-06-28
 *   info     : 
 * ======================================================== */

#ifndef _K2D_H
#define _K2D_H


/* ------------------------
 * gmm cluster in 2d data
 * x , input matrix
 * n , instance number
 * f , dim of data
 * k , cls count
 * assign, data cls assign
 * return, double (*) [5]
 *         cls infor
 *         x, y, xx, yy, xy
 * ------------------------ */
double (*k2d(double (*x)[2], int *assign, int n, int k))[6];


#endif //K2D_H
