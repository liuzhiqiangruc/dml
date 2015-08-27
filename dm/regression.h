/* ========================================================
 *   Copyright (C) 2015 All rights reserved.
 *   
 *   filename : regression.h
 *   author   : liuzhiqiang01@baidu.com
 *              lizeming@baidu.com
 *   date     : 2015-08-27
 *   info     : Using Regression.h to implement common 
 *              Regression task, include :
 *               1.logistical    regression
 *               2.possion       regression
 *               3.guassian      regression
 *               4.exponential   regression
 * ======================================================== */

#ifndef _REGRESSION_H
#define _REGRESSION_H

#define  REGRESSION_LINE_LEN  1048576 
#define  REGRESSION_KEY_LEN   64

/* -----------------------
 * regression parameter
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
} RP;

/* -------------------------
 * Regression Dataset struct  
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
typedef struct {
    RDS   * train_ds;                     /* train dataset pointer    */
    RDS   *  test_ds;                     /* test  dataset pointer    */
    char (*id_map)[REGRESSION_KEY_LEN];   /* feature id map           */
    int     c;                            /* unique feature count     */
    double *x;                            /* learned weight result    */
    RP      p;                            /* Regression parameters    */
} Regression;

/* --------------------------
 * Regression Opeartion
 * -------------------------- */
Regression * create_regression();
int   init_regression(Regression * regression);
int  learn_regression(Regression * regression);
void  save_regression(Regression * regression, int n);
void  free_regression(Regression * regression);

#endif //REGRESSION_H
