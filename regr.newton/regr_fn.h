/* ========================================================
 *   Copyright (C) 2016 All rights reserved.
 *   
 *   filename : regr_fn.h
 *   author   : liuzhiqiangruc@126.com
 *   date     : 2016-01-12
 *   info     : regression function define
 * ======================================================== */

#ifndef _REGR_FN_H
#define _REGR_FN_H

typedef double (*EVAL_FN)(double *x, void *data);
typedef int    (*REPO_FN)(double *x0, double *x1, void *data);
typedef void   (*GRAD_FN)(double *x, void *data, double *g, double *sg);

#endif //REGRESS_FN_H
