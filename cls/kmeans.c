/* ========================================================
 *   Copyright (C) 2014 All rights reserved.
 *   
 *   filename : kmeans.c
 *   author   : liuzhiqiang01@baidu.com
 *   date     : 2014-09-19
 *   info     : implementation of kmeans++
 * ======================================================== */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "kmeans.h"


/* ****************************************
 * brief  : cal a rand int for 0 ~ d
 * d      : upper bound of rand
 * ****************************************/
static int randd(int d){
    return (int)(0.1 * rand() / (RAND_MAX + 1.0) * d);
}

/* ****************************************
 * brief  : cal a rand doubld for 0 ~ s
 * s      : upper bound of rand
 * ****************************************/
static double randf(double s){
    return (1.0 * rand() / (RAND_MAX + 0.1) * s);
}


/* **********************************************
 * brief   : the distance between two instance
 * feature1: features of instance 1
 * feature2: features of instance 2
 * f       : features dim
 * return  : o2 dist
 * **********************************************/
static double dist(double * feature1, double * feature2, int f){
    double d = 0.0;
    for (int i = 0; i < f; i ++){
        d += (feature1[i] - feature2[i]) * (feature1[i] - feature2[i]);
    }
    return sqrt(d);
}

/* **********************************************
 * brief  : find nearest cent from c cents with s
 * s      : the sample instance
 * cents  : the cluster cents
 * c      : No. of cents
 * f      : dim of instance and cents
 * dd     : the nearest distance
 * return : the nearest cent
 * **********************************************/
static int nearest(double * s, double * cents, int c, int f, double * dd){
    int rc = -1;
    double d = 1e12;
    for (int i = 0 ; i < c; i++){
        double t = dist(s, cents + i * f, f);
        if (t < d){d = t; rc = i;}
    }
    if (dd) *dd = d;
    return rc;
}

/* *****************************************
 * brief  : init cents for kmeans++
 * m      : the data matrix n * f
 * n      : data size
 * f      : dim of data
 * k      : No. of cents
 * cents  : the cluster cents
 * centsA : the feature sum of instances in each cent
 * centsC : the instance count of each cent
 * cids   : the cent id of each instance
 * *****************************************/
static int init_cents(double * m, int n, int f, int k, double * cents, double * centsA, int * centsC, int * cids){
    int b = randd(n);
    double * d = (double*)malloc(sizeof(double) * n);
    memset(d, 0, sizeof(double) * n);

    memmove(cents, m + b * f, sizeof(double) * f);

    for (int c = 1; c < k; c++){
        memset(d,0,sizeof(double) * n);
        for (int i = 0; i < n; i++){
            nearest(m + i * f, cents, c, f, d + i);
            if (i > 0) d[i] += d[i-1];
        }
        double s = randf(d[n - 1]);
        for (int i = 0; i < n ; i++){
            if (d[i] > s){
                memmove(cents + c * f, m + i * f, sizeof(double) * f);
                break;
            }
        }
    }

    free(d); d = NULL;

    for (int i = 0; i< n; i++){
        int cent = nearest(m + i * f, cents, k, f , NULL);
        cids[i] = cent;
        centsC[cent] += 1;
        for (int j = 0; j < f; j++){
            centsA[cent * f + j] += m[i * f + j];
        }
    }

    for (int i = 0; i < k; i ++){
        for (int j = 0; j < f; j++){
            cents[i * f + j] = centsA[i * f + j] / centsC[i];
        }
    }

    return 0;
}

/* *************************************************
 * brief  : kmeans++ algorithm
 * m      : the data matrix
 * n      : data size
 * f      : dim of data
 * k      : No. of cents
 * c      : the cent id for each instance
 * *************************************************/
int kmeans(double * m, int n, int f, int k, int * c){
    double * cents  = (double*) malloc(sizeof(double) * k * f);
    double * centsA = (double*) malloc(sizeof(double) * k * f);
    int   * centsC = (int*)malloc(sizeof(int) * k);

    memset(cents, 0,sizeof(double) * k * f);
    memset(centsA,0,sizeof(double) * k * f);
    memset(centsC,0,sizeof(int) * k);

    init_cents(m, n, f, k, cents, centsA, centsC, c);

    int niter = 0;
    while (niter <  100){
        int update = 0;
        for (int i = 0 ; i < n; i ++){
            int oldcent = c[i];
            int newcent = nearest(m + i * f, cents, k, f, NULL);
            if (newcent != oldcent) if (centsC[oldcent] > 2) {
                update += 1;
                centsC[oldcent] -= 1;
                centsC[newcent] += 1;
                for (int j = 0; j < f; j++){
                    centsA[oldcent * f + j] -= m[i * f + j];
                    centsA[newcent * f + j] += m[i * f + j];
                }
                c[i] = newcent;
            }
        }

        for (int i = 0; i < k; i++){
            for (int j = 0; j < f; j++){
                cents[i * f + j] = centsA[i * f + j] / centsC[i];
            }
        }
        
        if (update <= n>>7){
            break;
        }
        niter += 1;
        fprintf(stderr,"kmeans iteration: %d, change instance : %d\n", niter,update);
    }

    free(cents);  cents = NULL;
    free(centsA); centsA = NULL;
    free(centsC); centsC = NULL;

    return 0;
}

