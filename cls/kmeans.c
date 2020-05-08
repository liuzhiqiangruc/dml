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
#include <unistd.h>
#include "rand.h"
#include "kmeans.h"

#ifndef ZERO
#define ZERO 1000
#endif

typedef struct _thread_arg{
    double *m;
    double *cents;
    double *dis;
    int * upd;
    int * cids;
    int * itst;
    int k;
    int n;
    int f;
    int tid;
    int ths;
} ThreadArg;

/* **********************************************
 * brief   : the distance between two instance
 * **********************************************/
static double dist(double * feature1, double * feature2, int f){
    double d = 0.0;
    for (int i = 0; i < f; i ++){
        d += (feature1[i] - feature2[i]) * (feature1[i] - feature2[i]);
    }
    return d;
}

/* **********************************************
 * brief  : find nearest cent from c cents with s
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
 * *****************************************/
static int init_cents(double * m, int n, int f, int k, double * cents, int c, int * cids, double * d, double inithe){
    RInfo * rinfo = create_rinfo(4357U + time(NULL));
    int sampled_i, b;
    double t = 0.0;
    if (c > k || c < 0){
        fprintf(stderr, "init error\n");
        return -1;
    }
    double *cd = (double*)malloc(sizeof(double) * n);
    if (c == 0){ // first center randome selected
        b = (int) (randomMT(rinfo) / (RAND_MAX + 1.0) * n);
        memmove(cents, m + b * f, sizeof(double) * f);
        memset(cids, 0, sizeof(int) * n);
        for (int i = 0; i < n; i++){
            d[i] = dist(m + i * f, cents, f);
        }
        c = 1;
    }
    // there are "c" inited centers already , must less than k
    else if (c > 0) for (int i = 0; i < n; i++){
        cids[i] = nearest(m + i * f, cents, c, f, d + i);
    }
    for (int i = 0; i < n; i++){
        cd[i] = (d[i] >= inithe ? d[i] : 0.0);
        if (i > 0) cd[i] += cd[i - 1];
    }
    // cluster growing from c to k at most
    for (; c < k; c++){
        t = randomMT(rinfo) / (RAND_MAX + 1.0) * cd[n - 1];
        sampled_i = binary_search(cd, n, t);
#ifdef DEBUG
        fprintf(stderr, "%4d : %16.3f %16.3f %12d\n", c, cd[n - 1], t, sampled_i);
#endif
        if (cd[n - 1] <=  ZERO) break;
        memmove(cents + c * f, m + sampled_i * f, sizeof(double) * f);
        for (int i = 0; i < n; i++){
            t = dist(m + i * f,  cents + c * f, f);
            if (t < d[i]) {d[i] = t; cids[i] = c;}
            cd[i] = (d[i] >= inithe ? d[i] : 0.0);
            if (i > 0) cd[i] += cd[i - 1];
        }
    }
    fprintf(stderr, "instance : %d, generated cluster K: %d\n", n, c);
    free(cd); cd = NULL;
    free(rinfo); rinfo = NULL;
    return c;
}

/* *************************************************
 * brief  : estep thread call
 * *************************************************/
static void * estep_thread_call (void * arg){
    ThreadArg * TH = (ThreadArg*)arg;
    double *m     = TH->m;
    double *cents = TH->cents;
    double *dis   = TH->dis;
    int    *upd   = TH->upd;
    int    *cids  = TH->cids;
    int    *itst  = TH->itst;
    int     k     = TH->k;
    int     f     = TH->f;
    int     n     = TH->n;
    int     tid   = TH->tid;
    int     ths   = TH->ths;
    int     l     = n / ths;
    int     s     = n % ths;
    int     begin = l * tid + (tid <= s ? tid : s);
    int     end   = l * (tid + 1) + ((tid + 1) <= s ? (tid + 1) : s);
    int     e = 0;
    while (1) {
        if (*itst == 0){
            *upd = 0;
            for (int i = begin; i < end; i++){
                int old = cids[i];
                cids[i] = nearest(m + i * f, cents, k, f, dis + i);
                if (dis[i] > 0.8) cids[i] = -1;
                if (old != cids[i]) *upd += 1;
            }
            *itst = (++e);
        }
        else if (*itst == -1){
            break;
        }
        else {
            sleep(1);
        }
    }
    return NULL;
}

/* *************************************************
 * brief  : mstep thread call
 * *************************************************/
static void m_step_call(double *m, double *cents, int *c, int n, int f, int k, int * outlier){
    double * centsA = (double*)calloc(sizeof(double), k * f);
    int    * centsC = (int*)   calloc(sizeof(int),    k);
    double   centsL2Norm = 0.0;
    *outlier = n;
    for (int i = 0; i< n; i++) if (c[i] >= 0) {
        *outlier -= 1;
        centsC[c[i]] += 1;
        for (int j = 0; j < f; j++){
            centsA[c[i] * f + j] += m[i * f + j];
        }
    }
    for (int i = 0; i < k; i ++) if (centsC[i] > 0){
        centsL2Norm = 0.0;
        for (int j = 0; j < f; j++){
            cents[i * f + j] = centsA[i * f + j] / centsC[i];
            centsL2Norm += cents[i * f + j] * cents[i * f + j];
        }
        centsL2Norm = sqrt(centsL2Norm);
        for (int j = 0; j < f; j++){
            cents[i * f + j] /= centsL2Norm;
        }
    }
    free(centsA); centsA = NULL;
    free(centsC); centsC = NULL;
}

/* *************************************************
 * brief  : kmeans++ algorithm
 * *************************************************/
int kmeans(double * m, int n, int f, int k, int initk, double * cents, int * c, double * dis, int ths, int maxiter, double inithe){
    int niters = 0, update = 0, outlier = 0;
    if (initk < 0 || initk > k) return -1;
    if (initk < k){ // generate k - initk new cluster at most
        memset(cents + initk * f, 0, sizeof(double) * (k - initk) * f);
        k = init_cents(m, n, f, k, cents, initk, c, dis, inithe);
    }
    // no need "else", no new cluster generted even through init_cents process!!
    if (initk == k) fprintf(stderr, "Just Tunning, No New Cluster Generated\n");
    int thread_update[32] = {0};  // 32 thread at most
    int iter_process[32] = {0};
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
        args[i].itst  = iter_process  + i;
        args[i].cids  = c;
        args[i].ths   = ths;
        args[i].dis   = dis;
    }
    m_step_call(m, cents, c, n, f, k, &outlier);
    for (int i = 0; i < ths; i++){
        pthread_create(tids + i, NULL, estep_thread_call, args + i);
    }
    while (1){
        int wait = 0;
        for (int i = 0; i < ths; i++){
            if ((niters +1) != iter_process[i]){
                wait = 1;
                break;
            }
        }
        if (wait == 0){
            // No. of instance whose clsid changed
            update = 0;
            for (int i = 0; i < ths; i++){
                update += thread_update[i];
            }
            fprintf(stderr,"kmeans iteration: %3d, change instance : %6d, outlier: %d\n", niters, update, outlier);
            if (update <= n / 256){
                break;
            }
            m_step_call(m, cents, c, n, f, k, &outlier);
            for (int i = 0; i < ths; i++){
                iter_process[i] = 0;
            }
            niters += 1;
            if (niters >= maxiter) {
                break;
            }
        }
    }
    for (int i = 0; i < ths; i++){
        iter_process[i] = -1;
    }
    for (int i = 0; i < ths; i++){
        pthread_join(tids[i], NULL);
    }
    return k;
}
