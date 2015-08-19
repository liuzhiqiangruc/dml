/* ========================================================
 *   Copyright (C) 2014 All rights reserved.
 *   
 *   filename : fn_type.h
 *   author   : liuzhiqiang01@baidu.com
 *   date     : 2014-12-23
 *   info     : common function defination
 * ======================================================== */

#ifndef _FN_TYPE_H
#define _FN_TYPE_H


typedef int    (*CMP_FN)(void *, void*);
typedef void   (*FREE_FN)(void *);


// lr call back functions
typedef double (*EVAL_FN)(double *x, void *data);
typedef int    (*REPO_FN)(double *x0, double *x1, void *data);
typedef void   (*GRAD_FN)(double *x, void *data, double *g);

#endif //FN_TYPE_H
