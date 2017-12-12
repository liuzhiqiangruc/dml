/* ========================================================
 *   Copyright (C) 2016 All rights reserved.
 *   
 *   filename : regress.h
 *   author   : liuzhiqiangruc@126.com
 *   date     : 2016-01-07
 *   info     : regression framework define
 * ======================================================== */

#ifndef _REGRESS_H
#define _REGRESS_H

#include "regr_fn.h"

#define RKL 256

/* --------------------------
 * parameters for regression
 * -------------------------- */
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
} RP;

/* -------------------------
 * Dataset for regression
 * ------------------------- */
typedef struct {
    int     r;                  /* instance num for train   */
    int    *l;                  /* feature cnt of instances */
    int    *ids;                /* all feature ids of train */
    double *val;                /* all feature val of train */
    double *y;                  /* target label of train    */
} RDS;

/* -------------------------
 * Regression Model struct 
 * ------------------------- */
typedef struct _reg {
    RDS   * train_ds;           /* train dataset pointer    */
    RDS   *  test_ds;           /* test  dataset pointer    */
    char  (*id_map)[RKL];       /* feature id map           */
    int     c;                  /* unique feature count     */
    double *x;                  /* learned weight result    */
    RP      p;                  /* Regression parameters    */
    EVAL_FN eval_fn;            /* loss function            */
    GRAD_FN grad_fn;            /* grandient function       */
    REPO_FN repo_fn;            /* report function          */
} REG;

/* --------------------------
 * Regression Opeartion
 * -------------------------- */
REG  * create_model(EVAL_FN eval_fn, GRAD_FN grad_fn, REPO_FN repo_fn);
int  init_model(REG * reg);
int  learn_model(REG * reg);
void save_model(REG * reg, int n);
void free_model(REG * reg);

#endif //REGRESS_H
