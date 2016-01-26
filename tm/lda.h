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
    int    niters;         /* iter n  */     
    int    savestep;       /* save s  */     
    int    k;              /* topic n */     
    char * in_file;        /* data in */     
    char * out_dir;        /* out dir */    
} ParamLda;

/* ---------------------------------------------------
 * matrix element for theta and phi
 * --------------------------------------------------- */
typedef struct {
    unsigned short prev;
    unsigned short next;
    unsigned int   count;
} ModelEle;

/* ----------------------------------------------------
 * model struct for Lda
 * ---------------------------------------------------- */
typedef struct _lda {
    int d;                         /* doc size          */
    int t;                         /* token size        */
    int v;                         /* dict size         */
    char (*id_doc_map)[KEY_SIZE];  /* doc id map        */
    char (*id_v_map)[KEY_SIZE];    /* word id map       */
    int  (*tokens)[4];             /* tokens            */
    int  * doc_entry;              /* doc entry         */
    ModelEle * nd;                 /* doc_topic matrix  */
    ModelEle * nw;                 /* topic_word matrix */
    int * nkw;                     /* token n of topic  */
    ParamLda p;                    /* lda parameter     */
} Lda;

/* --------------------------------
 * Operation for Lda
 * -------------------------------- */
Lda * create_lda();
int  init_lda(Lda * lda);
void  est_lda(Lda * lda);
void save_lda(Lda * lda,int n);
void free_lda(Lda * lda);

#endif //LDA_H
