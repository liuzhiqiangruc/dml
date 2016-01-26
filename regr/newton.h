/* ========================================================
 *   Copyright (C) 2016 All rights reserved.
 *   
 *   filename : newton.h
 *   author   : liuzhiqiangruc@126.com
 *   date     : 2016-01-07
 *   info     : newton method include lbfgs & owlqn
 * ======================================================== */

#ifndef _NEWTON_H
#define _NEWTON_H

#include "regr_fn.h"

typedef enum { OWLQN = 1, LBFGS } NT_METHOD;

/* -----------------------------------------
 * brief    :  newton method for l2 & l1 reg
 * eval_fn  :  function for loss function
 * grad_fn  :  grandient function for iter
 * repo_fn  :  report function after iter
 * data     :  dataset
 * max_iter :  max count for iteration
 * n        :  length of retx
 * retx     :  result x for data
 * ----------------------------------------- */
int newton(EVAL_FN eval_fn, GRAD_FN grad_fn, REPO_FN repo_fn, NT_METHOD method, void * data, int m, int max_iter, int n, double *retx);

#endif //NEWTON_H
