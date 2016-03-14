/* ========================================================
 *   Copyright (C) 2016 All rights reserved.
 *   
 *   filename : gbdt.h
 *   author   : liuzhiqiangruc@126.com
 *   date     : 2016-02-26
 *   info     : gradient boosting decision tree model
 * ======================================================== */

#ifndef _GBDT_H
#define _GBDT_H

#include "dtree.h"


/* --------------------------------------
 * brief  : f' and f" function definition
 * f      : current model value
 * y      : target label
 * g      : f' outer
 * h      : f" outer
 * n      : length of f,y,g,h
 * -------------------------------------- */
typedef void(*G)(double * f, double * y, double * g, int n);
typedef void(*H)(double * f, double * y, double * h, int n);

#endif //GBDT_H

