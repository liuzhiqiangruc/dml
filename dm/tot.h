/* ========================================================
 *   Copyright (C) 2015 All rights reserved.
 *   
 *   filename : tot.h
 *   author   : liuzhiqiang01@baidu.com
 *   date     : 2015-04-27
 *   info     : implementation of topics over time
 * ======================================================== */

#ifndef _TOT_H
#define _TOT_H

#define KEY_SIZE 64

/* ----------------------------------
 * parameter struct for TOT
 * ---------------------------------- */
typedef struct {           
    double a;              /* alpha   */     
    double b;              /* beta    */     
    int niters;            /* iter n  */     
    int savestep;          /* save s  */     
    int k;                 /* topic n */     
    char * in_file;        /* data in */     
    char * out_dir;        /* out dir */    
} ParamTOT;

/* ----------------------------------------------------
 * model struct for TOT
 * ---------------------------------------------------- */
typedef struct {
    int d;                         /* doc size          */
    int t;                         /* token size        */
    int v;                         /* dict size         */
    char (*id_doc_map)[KEY_SIZE];  /* doc id map        */
    char (*id_v_map)[KEY_SIZE];    /* word id map       */
    int  (*tokens)[4];             /* tokens            */
    int * nd;                      /* doc-topic matrix  */
    int * nw;                      /* topic-word matrix */
    int * nkw;                     /* token n of topic  */
    double (*beta)[5];             /* beta value paras  */
    double *tk_time;               /* token time info   */
    ParamTOT p;                    /* lda parameter     */
} TOT;

/* --------------------------------
 * Operation for TOT
 * -------------------------------- */
TOT * create_tot();
int  init_tot(TOT * tot);
void  est_tot(TOT * tot);
void save_tot(TOT * tot, int n);
void free_tot(TOT * tot);


#endif //TOT_H
