/* ========================================================
 *   Copyright (C) 2015 All rights reserved.
 *   
 *   filename : bri_lda.h
 *   author   : liuzhiqiang01@baidu.com
 *   date     : 2015-04-02
 *   info     : the implementation of link-lda using gibbs
 * ======================================================== */

#ifndef _BRI_LDA_H
#define _BRI_LDA_H

#define KEY_SIZE 64


/* -------------------------------------
 * parameter sutrct for bridge lda
 * ------------------------------------- */
typedef struct {
    double a;
    double b1;
    double b2;
    int niters;
    int savestep;
    int k;
    char * in_file;
    char * out_dir;
} ParamBriLda;


/* ------------------------------------------
 * model struct for bridge lda
 * ------------------------------------------ */
typedef struct {
    int d;
    int t;
    int v1;
    int v2;
    char (*id_doc_map)[KEY_SIZE];
    char (*id_v1_map)[KEY_SIZE];
    char (*id_v2_map)[KEY_SIZE];
    int  (*tokens)[4];
    int * nd;
    int * nw1;
    int * nw2;
    int * nkw1;
    int * nkw2;
    ParamBriLda p;
} BriLda;

/* -------------------------------
 * operation for bridge lda
 * ------------------------------- */
BriLda   * create_bri_lda();
int  init_bri_lda(BriLda * blda);
void  est_bri_lda(BriLda * blda);
void save_bri_lda(BriLda * blda, int n);
void free_bri_lda(BriLda * blda);

#endif //BRI_LDA_H
