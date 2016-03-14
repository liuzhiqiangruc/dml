/* ========================================================
 *   Copyright (C) 2016 All rights reserved.
 *   
 *   filename : dtree.h
 *   author   : liuzhiqiangruc@126.com
 *   date     : 2016-02-26
 *   info     : decision_regression_tree model
 *              given loss function
 * ======================================================== */

#ifndef _DTREE_H
#define _DTREE_H

#include "tdata.h"


typedef struct _t_node DTree;

/* -----------------------------------------
 * brief   : generate tree by data ds
 * ds      : input data
 * F       : current model 
 * n       : length of F ,g, h
 * ----------------------------------------- */
DTree * generate_dtree(DTrain * ds, double *F, double * g, double * h, int n);

#endif //DTREE_H
