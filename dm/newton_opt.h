/* ========================================================
 *   Copyright (C) 2014 All rights reserved.
 *
 *   filename : newton_opt.h
 *   author   : ***
 *   date     : 2015-7-21
 *   info     : newton optimization
 *              include:
 *              bfgs
 *              lbfgs
 *              owlqn
 * ======================================================== */

#ifndef _NEWTON_OPT_H
#define _NEWTON_OPT_H

#include <fn_type.h>


int bfgs(void *data, EVAL_FN eval_fn, GRAD_FN grad_fn, double ftol, int n, int it, double *retX);
int lbfgs(void *data, EVAL_FN eval_fn, GRAD_FN grad_fn, double ftol, int m, int n, int it, double *retX);
int owlqn(void *data, EVAL_FN eval_fn, GRAD_FN grad_fn, double l1, double ftol, int m, int n, int it, double *retX);


#endif

