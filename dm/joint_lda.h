/* ========================================================
 *   Copyright (C) 2015 All rights reserved.
 *   
 *   filename : joint_lda.h
 *   author   : ***
 *   date     : 2015-03-23
 *   info     : 
 * ======================================================== */

#ifndef _LDA_H
#define _LDA_H

#define KEY_SIZE 64

/* -----------------------------------
 * parameter struct for Joint Lda
 * ----------------------------------- */
typedef struct {
    double a;                /* alpha  */
    double a1;               /* alpha1 */
    double a2;               /* alpha2 */
    double b1;               /* beta1  */
    double b2;               /* beta2  */
    int niters;              /* iter n */
    int savestep;            /* step n */
    int k;                   /* K      */
    int k1;                  /* K1     */
    int k2;                  /* K2     */
    char * in_file;          /* data   */
    char * out_dir;          /* outdir */
} ParamJointLda;

/* ----------------------------------------------------------------- 
 * model struct for Joint Lda
 * ----------------------------------------------------------------- */
typedef struct {
    int d;                            /* doc num                     */
    int t;                            /* token num                   */
    int v1;                           /* dict size of data 1         */
    int v2;                           /* dict size of data 2         */
    char (*id_doc_map)[KEY_SIZE];     /* doc id map          -vector */
    char (*id_v1_map)[KEY_SIZE];      /* dict 1 id map       -vector */
    char (*id_v2_map)[KEY_SIZE];      /* dict 2 id map       -vector */
    int  (*tokens)[4];                /* tokens list         -matrix */
    int * nx;                         /* doc K list          -vector */
    int * kd;                         /* K doc num list      -vector */
    int * dk1;                        /* doc k1 distribute   -matrix */
    int * dk2;                        /* doc k2 distribute   -matrix */
    int * kk1;                        /* K K1 distribute     -matrix */
    int * kk2;                        /* K K2 distribute     -matrix */
    int * vk1;                        /* Dict1 K1 distribute -matrix */
    int * vk2;                        /* Dict2 K2 distribute -matrix */
    int * k1w;                        /* K1 dict1 distribute -vector */
    int * k2w;                        /* K2 dict2 distribute -vector */
    ParamJointLda p;
} JointLda;


/* ---------------------------------
 * Operator for Joint Lda
 * --------------------------------- */
JointLda * create_joint_lda();
int  init_joint_lda(JointLda * jlda);
void est_joint_lda (JointLda * jlda);
void save_joint_lda(JointLda * jlda, int n);
void free_joint_lda(JointLda * jlda);


#endif //LDA_H
