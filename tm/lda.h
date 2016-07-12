/* ========================================================
 *   Copyright (C) 2015 All rights reserved.
 *   
 *   filename : lda.h
 *   author   : liuzhiqiangruc@126.com
 *   date     : 2016-01-22
 *   info     : implementation of LDA
 * ======================================================== */

#ifndef _LDA_H
#define _LDA_H

#define KEY_SIZE 64

/* ----------------------------------
 * parameter struct for lda
 * ---------------------------------- */
typedef struct {           
    double a;              /* alpha   */     
    double b;              /* beta    */     
    double g0;             /* gamma0  */
    double g1;             /* gamma1  */
    int    niters;         /* iter n  */     
    int    savestep;       /* save s  */     
    int    k;              /* topic n */     
    char * in_file;        /* data in */     
    char * out_dir;        /* out dir */    
} ParamLda;

typedef struct _lda Lda;

/* --------------------------------
 * Operation for Lda
 * -------------------------------- */
Lda * create_lda(ParamLda p);
int  init_lda(Lda * lda);
void  est_lda(Lda * lda);
void save_lda(Lda * lda,int n);
void free_lda(Lda * lda);
int  lda_d(Lda * lda);
int  lda_v(Lda * lda);
int  lda_t(Lda * lda);

#endif //LDA_H
