/* ========================================================
 *   Copyright (C) 2016 All rights reserved.
 *   
 *   filename : gblr.h
 *   author   : liuzhiqiangruc@126.com
 *   date     : 2016-02-26
 *   info     : 0-1 binomial log loss regression 
 *              using grandient boosting
 *              based on decision tree as base learner
 * ======================================================== */

#ifndef _GBLR_H
#define _GBLR_H

#include "gbdt.h"

GBDT * gbdt_lr(GBMP p);

#endif //GBLR_H
