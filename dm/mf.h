/* ========================================================
 *   Copyright (C) 2015 All rights reserved.
 *   
 *   filename : mf.h
 *   author   : liuzhiqiang01@baidu.com
 *   date     : 2015-05-25
 *   info     : matrix factorization framework
 * ======================================================== */

#ifndef _MF_H
#define _MF_H

#define KEY_SIZE 64

#define shuffle(array,n) do{                         \
    int *tmp = array;                                \
    int i,t,k;                                       \
    i = t = 0;                                       \
    for (k = n; k >= 2; k--){                        \
        i = (int)(rand() / (RAND_MAX + 1.0) * k);    \
        t = tmp[i]; tmp[i] = tmp[0]; *tmp++ = t;     \
    }                                                \
}while(0);

typedef struct {
    double a;
    double b;
    int niters;
    int nbias;
    int savestep;
    int k;
    char * in_file;
    char * te_file;
    char * out_dir;
} ParaMF;

typedef struct {
    int U;
    int I;
    int T;
    int TT;
    char (*id_u_map)[KEY_SIZE];
    char (*id_i_map)[KEY_SIZE];
    int  (*u_i)[2];
    int  (*u_t)[2];
    double *s;
    double *s_t;

    double *pu_tmp;
    double *qi_tmp;
    double *bu_tmp;
    double *bi_tmp;

    double *pu;
    double *qi;
    double *bu;
    double *bi;

    double mu;
    double min_s;
    double max_s;

    ParaMF p;
} MF;

MF * create_mf();
int  init_mf(MF * mf);
void  backup(MF * mf);
void recover(MF * mf);
void  est_mf(MF * mf);
void save_mf(MF * mf, int n);
void free_mf(MF * mf);


#endif //MF_H
