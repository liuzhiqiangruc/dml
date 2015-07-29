/* ========================================================
 *   Copyright (C) 2015 All rights reserved.
 *   
 *   filename : author_lda.h
 *   author   : liuzhiqiang01@baidu.com
 *   date     : 2015-04-02
 *   info     : implementation of author topic model
 * ======================================================== */

#ifndef _AUTHOR_LDA_H
#define _AUTHOR_LDA_H

#define KEY_SIZE 64

/* --------------------------------------------------------------
 * parameter struct for author topic model
 * -------------------------------------------------------------- */
typedef struct {                     
    double a;                          /* alpha                   */
    double b;                          /* beta                    */
    int k;                             /* topics                  */
    int niters;                        /* iters n                 */
    int savestep;                      /* model save per savestep */
    char * author_file;                /* author file of doc      */
    char * in_file;                    /* input file              */
    char * out_dir;                    /* model save dir          */
    char * phi_file;                   /* phi matrix input        */
} ParamALda;

/* --------------------------------------------------------------
 * author struct for one document
 * -------------------------------------------------------------- */
typedef struct {
    int nda;                           /* author cnt of doc       */
    int csum;                          /* sum of author cnt       */ 
    int (*das)[2];                     /* authors of doc          */
} DocAuthors;

/* --------------------------------------------------------------
 * model struct for author topic model
 * -------------------------------------------------------------- */
typedef struct {
    int D;                             /* doc num of copus        */
    int T;                             /* token num of copus      */
    int V;                             /* dict size of copus      */
    int A;                             /* totle author number     */
    int MA;                            /* max author count of doc */
    char (*id_doc_map)[KEY_SIZE];      /* doc id map              */
    char (*id_v_map)[KEY_SIZE];        /* word id map             */
    char (*id_au_map)[KEY_SIZE];       /* author id map           */
    DocAuthors * doc_author;           /* authors of docs         */
    int (*tokens)[4];                  /* doc tokens              */
    int * na;                          /* author topic matrix     */
    int * nw;                          /* topic word matrix       */
    int * nka;                         /* topic sumcnt of authors */
    int * nkw;                         /* word sumcnt of topics   */
    ParamALda p;                       /* param of author topic m */
} AuthorLda;

/* ---------------------------------------
 * operation for author topic model
 * --------------------------------------- */
AuthorLda * create_author_lda();
int    init_author_lda(AuthorLda * alda);
void    est_author_lda(AuthorLda * alda);
void   save_author_lda(AuthorLda * alda, int n);
void   free_author_lda(AuthorLda * alda);


#endif //AUTHOR_LDA_H
