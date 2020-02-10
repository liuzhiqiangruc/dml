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
#include <time.h>
#include <pthread.h>
#include "rand.h"
#include "kmeans.h"

typedef struct _thread_arg{
    double *m;
    double *cents;
    int * upd;
    int * cids;
    int k;
    int n;
    int f;
    int tid;
    int ths;
} ThreadArg;

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

/* **************************************************
 * find the index i which v[i] > s and v[i - 1] <= s
 * **************************************************/
static int binary_search(double *v, int n, double s){
    int l, h, m;
    if (n < 0) {return -1;}
    if (n == 0 || v[0] > s){ return 0;}
    if (v[n - 1] <= s) {return n - 1;}
    l = 0, h = n;
    while (h > l) {
        if (h == l + 1){
            return (v[l] > s) ? l : h;
        }
        m = (h + l) / 2;
        if (v[m] > s && v[m - 1] <= s){
            return m;
        }
        else if (v[m] <= s){
            l = m + 1;
        }
        else if (v[m - 1] > s){
            h = m - 1;
        }
    }
    return h;
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
    RInfo * rinfo = create_rinfo(4357U + time(NULL));
    int sampled_i, b;
    double t = 0.0;
    double * d = (double*)malloc(sizeof(double) * n);
    double *cd = (double*)malloc(sizeof(double) * n);
    // first center randome selected
    b = (int) (randomMT(rinfo) / (RAND_MAX + 1.0) * n);
    memmove(cents, m + b * f, sizeof(double) * f);
    for (int i = 0; i < n; i++){
        d[i] = dist(m + i * f, cents, f);
        cd[i] = d[i];
        if (i > 0){
            cd[i] += cd[i - 1];
        }
    }
    memset(cids, 0, sizeof(int) * n);
    for (int c = 1; c < k; c++){
        t = randomMT(rinfo) / (RAND_MAX + 0.1) * cd[n - 1];
        sampled_i = binary_search(cd, n, t);
        memmove(cents + c * f, m + sampled_i * f, sizeof(double) * f);
        for (int i = 0; i < n; i++){
            t = dist(m + i * f,  cents + c * f, f);
            if (t < d[i]) {d[i] = t; cids[i] = c;}
            cd[i] = d[i];
            if (i > 0){ cd[i] += cd[i - 1]; }
        }
    }
    free(d); d = NULL;
    free(cd); cd = NULL;
    free(rinfo); rinfo = NULL;
    for (int i = 0; i< n; i++){
        centsC[cids[i]] += 1;
        for (int j = 0; j < f; j++){
            centsA[cids[i] * f + j] += m[i * f + j];
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
 * brief  : estep thread call
 * *************************************************/
static void * estep_thread_call (void * arg){
    ThreadArg * TH = (ThreadArg*)arg;
    double *m     = TH->m;
    double *cents = TH->cents;
    int    *upd   = TH->upd;
    int    *cids  = TH->cids;
    int     k     = TH->k;
    int     f     = TH->f;
    int     n     = TH->n;
    int     tid   = TH->tid;
    int     ths   = TH->ths;
    int     l     = n / ths;
    int     s     = n % ths;
    int     begin = l * tid + (tid <= s ? tid : s);
    int     end   = l * (tid + 1) + ((tid + 1) <= s ? (tid + 1) : s);
    *upd = 0;
    for (int i = begin; i < end; i++){
        int old = cids[i];
        cids[i] = nearest(m + i * f, cents, k, f, NULL);
        if (old != cids[i]) *upd += 1;
    }
    return NULL;
}

/* *************************************************
 * brief  : kmeans++ algorithm
 * m      : the data matrix
 * n      : data size
 * f      : dim of data
 * k      : No. of cents
 * c      : the cent id for each instance
 * *************************************************/
int kmeans(double * m, int n, int f, int k, int * c, int ths){
    int niters = 0, update = 0;
    double * cents  = (double*) malloc(sizeof(double) * k * f);
    double * centsA = (double*) malloc(sizeof(double) * k * f);
    int    * centsC = (int*)malloc(sizeof(int) * k);
    memset(cents, 0,sizeof(double) * k * f);
    memset(centsA,0,sizeof(double) * k * f);
    memset(centsC,0,sizeof(int) * k);
    init_cents(m, n, f, k, cents, centsA, centsC, c);
    int thread_update[32] = {0};
    pthread_t tids[32] = {0};
    ThreadArg args[32] = {{0}};
    if (ths <=1 ) ths = 1;
    if (ths >= 32) ths = 32;
    for (int i = 0; i < ths; i++){
        args[i].m     = m;
        args[i].cents = cents;
        args[i].k     = k;
        args[i].n     = n;
        args[i].f     = f;
        args[i].tid   = i;
        args[i].upd   = thread_update + i;
        args[i].cids  = c;
        args[i].ths   = ths;
    }
    while (++niters  <= 100){
        update = 0;
        for (int i = 0; i < ths; i++){
            pthread_create(tids + i, NULL, estep_thread_call, args + i);
        }
        for (int i = 0; i < ths; i++){
            pthread_join(tids[i], NULL);
        }
        for (int i = 0; i < ths; i++){
            update += thread_update[i];
        }
        memset(centsA,0,sizeof(double) * k * f);
        memset(centsC,0,sizeof(int) * k);
        for (int i = 0; i< n; i++){
            centsC[c[i]] += 1;
            for (int j = 0; j < f; j++){
                centsA[c[i] * f + j] += m[i * f + j];
            }
        }
        for (int i = 0; i < k; i ++){
            for (int j = 0; j < f; j++){
                cents[i * f + j] = centsA[i * f + j] / centsC[i];
            }
        }
        fprintf(stderr,"kmeans iteration: %d, change instance : %d\n", niters, update);
        if (update <= n / 256){
            break;
        }
    }
    free(cents);  cents = NULL;
    free(centsA); centsA = NULL;
    free(centsC); centsC = NULL;
    return 0;
}