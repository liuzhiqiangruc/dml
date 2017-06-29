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

typedef struct _d_tree DTree;

/* -----------------------------------------
 * brief   : generate tree by data ds
 * ds      : input data
 * F       : current model 
 * g       : 1-gradient
 * h       : 2-gradient
 * n       : length of F ,g, h
 * s       : min instances for each node
 * d       : max depth of trees
 * m       : max leaf node in the tree
 * ----------------------------------------- */
DTree * generate_dtree(DTD * ds, double * F, double * g, double * h, double nr, double wr, int n, int s, int d, int m);

/* -------------------------
 * brief   : free tree space
 * ------------------------- */
void free_dtree(DTree * t);

/* ------------------------------------
 * brief   : predict ts with tree t
 * return  : predict value(score)
 * ------------------------------------ */
double * eval_tree(DTD * ts, DTree * t, double * F, int n);

/* ------------------------------------
 * brief   : save decision tree to file
 * ------------------------------------ */
void save_dtree(DTree * t, char * out_file, char (*id_map)[FKL]);

DTree * unserialize_dtree(DTree * dt, int n);

int serialize_dtree (DTree * t, DTree * rt);

size_t size_dtree(DTree * t);

#endif //DTREE_H
