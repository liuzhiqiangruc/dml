/* ========================================================
 *   Copyright (C) 2017 All rights reserved.
 *   
 *   filename : gbcfg.h
 *   author   : ***
 *   date     : 2017-07-19
 *   info     : 
 * ======================================================== */

#ifndef _GBCFG_H
#define _GBCFG_H
/* --------------------------------------
 * GBDT configurations 
 * -------------------------------------- */
typedef struct _gbdt_param {
    double rate;                 /* learning rate      */
    double nod_reg;              /* regular for node   */
    double wei_reg;              /* regular for weight */
    double max_margin;           /* max margin for rank using hinge loss*/
    int    max_leaf_nodes;       /* max leaf nodes     */
    int    max_trees;            /* tree capacity      */
    int    max_depth;            /* max tree depth     */
    int    min_node_ins;         /* min ins. for node  */
    int    binary;               /* 1 for binary       */
    int    pnc;                  /* pthread count      */
    char * train_init;           /* train init model   */
    char * train_input;          /* train input file   */
    char * test_init;            /* test  init model   */
    char * test_input;           /* test  input file   */
    char * out_dir;              /* output dir         */
} GBMP;

#endif //GBCFG_H
