/* ========================================================
 *   Copyright (C) 2015 All rights reserved.
 *   
 *   filename : mf.c
 *   author   : liuzhiqiang01@baidu.com
 *   date     : 2015-05-22
 *   info     : binary matrix factorization
 *              using stochastic gradient descent(sgd)
 * ======================================================== */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "mf.h"


static void fullfill_param(MF * mf){
    double tmp = sqrt(mf->p.k);
    for (int i = 0; i < mf->U * mf->p.k; i++){
        mf->pu[i] = ((0.1 + rand()) / (1.0 + RAND_MAX) ) / tmp;
    }
    for (int i = 0; i < mf->I * mf->p.k; i++){
        mf->qi[i] = ((0.1 + rand()) / (1.0 + RAND_MAX) ) / tmp;
    }
}

static double logll(MF * mf, int t){
    double tmp = 0.0;
    double likelihood = 0.0;
    if (t == 0){
        for (int i = 0; i < mf->T; i++){
            int uid = mf->u_i[i][0];
            int iid = mf->u_i[i][1];
            int uoff = uid * mf->p.k; 
            int ioff = iid * mf->p.k;
            double y   = mf->s[i];
            tmp = mf->bu[uid] + mf->bi[iid];
            for (int k = 0; k < mf->p.k; k++){
                tmp += mf->pu[uoff + k] * mf->qi[ioff + k];
            }
            double e = 1.0/(1.0 + exp(-tmp));  
            if (1.0 == y) likelihood += log(e);
            else likelihood += log(1-e);   
        }
        return exp(likelihood/mf->T);
    }
    else {
        for (int i = 0; i < mf->TT; i++){
            int uid = mf->u_t[i][0];
            int iid = mf->u_t[i][1];
            int uoff = uid * mf->p.k; 
            int ioff = iid * mf->p.k;
            double y   = mf->s_t[i];
            tmp = mf->bu[uid] + mf->bi[iid];
            for (int k = 0; k < mf->p.k; k++){
                tmp += mf->pu[uoff + k] * mf->qi[ioff + k];
            }
            double e = 1.0/(1.0 + exp(-tmp));  
            if (1.0 == y) likelihood += log(e);
            else likelihood += log(1-e);   
        }
        return exp(likelihood/mf->TT);
    }
}


void  est_mf(MF * mf) {
    fullfill_param(mf);
    double l_logll = logll(mf, 0);
    fprintf(stderr, "iter :0 train logll : %f   test logll: %f  learn rate : %f\n", l_logll, logll(mf, 1), mf->p.a);
    int *p = (int*)malloc(sizeof(int) * mf->T);
    for (int i = 0; i < mf->T; i++) p[i] = i;
    int n = 1;
    while (n <= mf->p.niters){
        fprintf(stderr, "iter :%d ", n);
        shuffle(p, mf->T);
        backup(mf);
        for (int j = 0; j < mf->T; j++){
            int id = p[j];
            int uid = mf->u_i[id][0];
            int iid = mf->u_i[id][1];
            int uoff = uid * mf->p.k;
            int ioff = iid * mf->p.k;
            double score = mf->s[id];
            double rx = mf->bu[uid] + mf->bi[iid];
            if (n > mf->p.nbias){
                for (int k = 0; k < mf->p.k; k++){
                    rx += mf->pu[uoff + k] * mf->qi[ioff + k];
                }
            }
            double e = 1.0/(1.0 + exp(-rx)); 
            e = score - e;

            mf->bu[uid] += mf->p.a * (e - mf->p.b * mf->bu[uid]);
            mf->bi[iid] += mf->p.a * (e - mf->p.b * mf->bi[iid]);
            if (n > mf->p.nbias){
                for (int k = 0; k < mf->p.k; k++){
                    double tmp = mf->pu[uoff + k];
                    mf->pu[uoff + k] += mf->p.a * (e * mf->qi[ioff + k] - mf->p.b * mf->pu[uoff + k]);
                    mf->qi[ioff + k] += mf->p.a * (e * tmp              - mf->p.b * mf->qi[ioff + k]);
                }
            }
        }
        double c_logll = logll(mf, 0);
        if (c_logll > l_logll){
            mf->p.a *= 0.99;
            l_logll = c_logll;
            n += 1;
            double v_logll = logll(mf, 1);
            fprintf(stderr, "train logll: %f   test logll: %f  learn rate : %f\n", c_logll, v_logll, mf->p.a);
            if (n % mf->p.savestep == 0){
                save_mf(mf, n);
            }
        }
        else{
            recover(mf);
            mf->p.a *= 0.8;
            fprintf(stderr, "run failed, try again\n");
        }
    }
    free(p); p = NULL;
}

