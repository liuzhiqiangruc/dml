/* ========================================================
 *   Copyright (C) 2017 All rights reserved.
 *   
 *   filename : fn_types.h
 *   author   : ***
 *   date     : 2017-07-19
 *   info     : 
 * ======================================================== */

#ifndef _FN_TYPES_H
#define _FN_TYPES_H

/* --------------------------------------
 * brief  : f', f" and report functions
 * f      : current model value
 * y      : target label
 * g      : f' outer
 * h      : f" outer
 * n      : length of f,y,g,h
 * -------------------------------------- */
typedef void(*G)(double * f, double * y, double * g, int n);
typedef void(*H)(double * f, double * y, double * h, int n);


#endif //FN_TYPES_H
