/* ========================================================
 *   Copyright (C) 2015 All rights reserved.
 *
 *   filename : lr.h
 *   author   : liuzhiqiang01@baidu.com
 *              lizeming@baidu.com
 *   date     : 2015-07-31
 *   info     : implementation of Logistic Regression 
 *              using newton method
 *              2015-8-7  add for binary features
 *              2015-8-14 add lr model wrapper
 * ======================================================== */

#ifndef _LR_H
#define _LR_H

#define KEY_SIZE 64

/* -----------------------
 * parameter struct for LR
 * ----------------------- */
typedef struct {
    double lambda;              /* penalty for regulization */
    double ftoler;              /* tolerance for conv       */
    int    niters;              /* max iters                */
    int    savestep;            /* save step                */
    int    iterno;              /* current iter number      */
    int    binary;              /* binary feature or not    */
    int    method;              /* regualization mod 1 or 2 */
    char  *in_file;             /* input file for train     */
    char  *te_file;             /* input file for test      */
    char  *out_dir;             /* output dir               */
} LRParam;

/* -------------------------
 * LR Dataset struct  
 * ------------------------- */
typedef struct {
    int     r;                  /* instance num for train   */
    int    *l;                  /* feature cnt of instances */
    int    *ids;                /* all feature ids of train */
    double *val;                /* all feature val of train */
    double *y;                  /* target label of train    */
} LRDS;

/* -------------------------
 * LR Model struct 
 * ------------------------- */
typedef struct {
    LRDS  * train_ds;           /* train dataset pointer    */
    LRDS  *  test_ds;           /* test  dataset pointer    */
    char (*id_map)[KEY_SIZE];   /* feature id map           */
    int     c;                  /* unique feature count     */
    double *x;                  /* learned weight result    */
    LRParam p;                  /* LR parameters            */
} LR;

/* --------------------------
 * LR Opeartion
 * -------------------------- */
LR * create_lr_model();
int   init_lr(LR * lr);
int  learn_lr(LR * lr);
void  save_lr(LR * lr, int n);
void  free_lr(LR * lr);

#endif //LR
