/* ========================================================
 *   Copyright (C) 2014 All rights reserved.
 *   
 *   filename : modl.c
 *   author   : liuzhiqiang01@baidu.com
 *   date     : 2014-10-17  v1.0
 *              2015-04-30  1. update the greed init function
 *                             and modify the LogD method
 *                          2. update exMerge function
 *                             no recalculation function value
 *                             at line 193
 *   info     : implementation for the modl algorithm
 * ======================================================== */

#include <math.h>
#include <stdlib.h>
#include <string.h>
#include "modl.h"

//S,E,N,C,C0,C1
typedef int Interval[6];

/* ------------------------------------------
 * brief : merge two intervals to one
 * Interval a: left interval 
 * Interval b: right interval
 * ------------------------------------------ */
static void mergeInterval(Interval a, Interval b){
    a[1] = b[1];
    a[2] += b[2];
    a[4] += b[4];
    a[5] += b[5];
    a[3] = 0;
    if (a[4] > 0) a[3] += 1;
    if (a[5] > 0) a[3] += 1;
}

/* ------------------------------------------------------------
 * brief    : init the Interval Array list using greed algorithm
 * double v : the value array
 * int    a : the label array
 * int    n : the array length
 * int   *m : the interval count
 * return   : greedInited Intervals
 * ------------------------------------------------------------ */
static Interval * greedInit(double v[], int a[], int n, int* m){
    int k = 1, c = 1;
    for (int i = 1; i < n; i++){
        if (v[i] != v[i - 1]) {
            k += 1;
        }
    }
    Interval * I = (Interval*)malloc(sizeof(Interval) * k);
    memset(I, 0, sizeof(Interval) * k);

    I[0][0] = 0; I[0][2] = 1;
    if (a[0] == 0) I[0][4] = 1;
    if (a[0] == 1) I[0][5] = 1;

    k = 0;
    for (int i = 1; i < n; i++){
        if (v[i - 1] == v[i]){
            I[k][2] += 1;
            if (a[i] == 0) I[k][4] += 1;
            if (a[i] == 1) I[k][5] += 1;
        }
        else {
            I[k][1] = i - 1;
            I[k][3] = 0;
            if (I[k][4] > 0) I[k][3] += 1;
            if (I[k][5] > 0) I[k][3] += 1;
            k += 1;
            I[k][0] = i;
            I[k][2] = 1;
            if (a[i] == 0) I[k][4] = 1;
            if (a[i] == 1) I[k][5] = 1;
        }
    }
    I[k][1] = n - 1;
    I[k][3] = 0;
    if (I[k][4] > 0) I[k][3] += 1;
    if (I[k][5] > 0) I[k][3] += 1;
    k += 1;

    for (int i = 1; i < k; i++){
        if (I[i-1][3] == 2 || I[i][3] == 2 || \
           (I[i-1][4] > 0  && I[i][5] > 0) || \
           (I[i-1][5] > 0  && I[i][4] > 0)) {
            c += 1;
        }
    }

    Interval * M = (Interval*)malloc(sizeof(Interval) * c);
    memset(M, 0, sizeof(Interval) * c);
    memmove(M, I, sizeof(Interval));

    c = 0;
    for (int i = 1; i < k; i++){
        if (I[i-1][3] == 2 || I[i][3] == 2 || \
           (I[i-1][4] > 0  && I[i][5] > 0) || \
           (I[i-1][5] > 0  && I[i][4] > 0)) {
            c += 1;
            memmove(M + c, I + i, sizeof(Interval));
        }
        else{
            M[c][1]  = I[i][1];
            M[c][2] += I[i][2];
            M[c][4] += I[i][4];
            M[c][5] += I[i][5];
        }
    }
    c += 1; *m = c;
    free(I); I = NULL;

    return M;

}

/* ---------------------------------------------------------------------
 * brief     : calculate the Delta value for combine two interval to one
 * Interval a: left  interval
 * Interval b: right interval
 * Interval c: combined interval
 * int m     : number of current intervals
 * int n     : number of elements
 * return    : delta value 
 * -------------------------------------------------------------------- */
static double getDelta(Interval a, Interval b, Interval c, int m, int n, double * LogD){

    double v = 0.0;
    v += LogD[m + n - 1] - LogD[m - 1] - LogD[m + n - 2] + LogD[m - 2];
    v += LogD[a[2] + 1] + LogD[b[2] + 1] - LogD[c[2] + 1];
    v += LogD[c[4]] + LogD[c[5]];
    v -= (LogD[a[4]] + LogD[a[5]]);
    v -= (LogD[b[4]] + LogD[b[5]]);
    return v;

}

/* -------------------------------------------
 * brief : get value for the current intervals
 * int m : number of current intervals
 * int n : number of elements
 * return : optimized function value
 * ------------------------------------------- */
static double getValue(Interval * t,int m, int n, double * LogD){
    int i = 0;
    double v = 0.0;
    v += LogD[n + m - 1] - LogD[m - 1];
    v -= LogD[n];
    for (i = 0; i < m; i++){
        v += LogD[t[i][2] + 1];
        v -= LogD[t[i][4]];
        v -= LogD[t[i][5]];
    }
    return v;

}

/* ---------------------------------------------------------------
 * brief :  ex merge the intervals to find the best merge solution
 * int m :  number of current intervals
 * int n :  number of elements
 * int *l:  the number of intervals after best merge 
 * return:  Intervals after best merge
 * --------------------------------------------------------------- */
static Interval * exMerge(Interval * I, int m, int n, int *l, double * LogD){
    int i = 0, u = 0;
    double d, maxd, p, minp;
    Interval *t = NULL, *to = NULL, ti;

    t = (Interval*) malloc(sizeof(Interval) * m);
    memmove(t,I,sizeof(Interval) * m);

    to = (Interval*) malloc(sizeof(Interval) * m);
    memmove(to,t,sizeof(Interval) * m);
    minp = p = getValue(t, m, n, LogD);
    *l = m;

    while (m > 1){
        // find the best merge intervals
        // which can get biggest delta
        maxd = -10e12;
        for (i = 0; i < m - 1; i++){
            memmove(ti,t[i],sizeof(Interval));
            mergeInterval(ti,t[i + 1]);
            d = getDelta(t[i],t[i+1],ti,m,n,LogD);
            if (d > maxd){
                maxd = d;
                u = i;
            }
        }

        // merge the two intervals which get the biggest delta
        // even when delta is less the 0
        mergeInterval(t[u],t[u+1]);
        if (u < m - 2){
            memmove(t[u + 1], t[u+2], sizeof(Interval) * (m - u - 2));
        }
        m -= 1;

        // current optimized function value
        p -= maxd;

        // if value is less 
        // store the better result
        if (p < minp){
            minp = p;
            memmove(to, t, sizeof(Interval) * m);
            *l = m;
        }
    }

    free(t); t= NULL;

    return to;
}

/* -------------------------------------------------
 * brief       : split one interval to two 
 * Interval sp1: splited interval 1
 * Interval sp2: splited interval 2
 * Interval * I: intervals after greedInit
 * int l       : the number of current Intervals
 * int m       : the number of greedinited Intervals I
 * int n       : the number of elements
 * return      : the delta after split t to sp1, sp2
 * ------------------------------------------------- */
static double splitInterval(Interval t, Interval sp1, Interval sp2, Interval * I, int l, int m, int n, double * LogD){
    int i; double mxspd,spd;
    Interval s1, s2;
    if (t[2] ==1 || t[3] == 1) return 0.0;

    for (i = 0; I[i][0] != t[0] && i< m; i++){}
    memmove(s1,I[i],sizeof(Interval));
    mxspd = -10e12;
    for(; I[i][1] != t[1] && i < m; i++){
        if (I[i][0] != s1[0] || I[i][1] != s1[1]){
            mergeInterval(s1,I[i]);
        }
        s2[0] = s1[1] + 1;
        s2[1] = t[1];
        s2[2] = t[1] - s1[1];
        s2[4] = t[4] - s1[4];
        s2[5] = t[5] - s1[5];
        s2[3] = 0;
        if (s2[4] > 0) s2[3] += 1;
        if (s2[5] > 0) s2[3] += 1;
        spd = -1.0 * getDelta(s1, s2, t, l + 1, n, LogD);
        if (spd > mxspd){
            mxspd = spd;
            memmove(sp1,s1,sizeof(Interval));
            memmove(sp2,s2,sizeof(Interval));
        }
    }
    return mxspd;
}

/* --------------------------------------------------------------
 * brief   : do split, merge split, merge merge split operation
 *           and select the best operation to do
 * Interval M : current Intervals
 * int l      : number of current Intervals
 * Interval I : greedinited Intervals
 * int m      : number of greedinited Intervals
 * int n      : number of elements
 * int *ol    : number of optimized Intervals, the final solution
 * return     : the delta of optimization
 * -------------------------------------------------------------- */
static double optimize(Interval * M, int l,  Interval * I, int m, int n, int *ol, double * LogD){
    int i,j,k;
    int spi = 0, msi = 0, mmsi = 0;
    double mxspd,spd,mxmsd,msd,mxmmsd,mmsd;
    Interval s1, s2, ss1, ss2, ms1, ms2, mms1, mms2, t, tt;

    // split operation
    mxspd = -10e12;
    for (i = 0; i< l; i++){
        memmove(t, M[i], sizeof(Interval));
        spd = splitInterval(t,s1,s2,I,l,m,n, LogD);
        if (spd > mxspd){
            mxspd = spd;
            memmove(ss1,s1,sizeof(Interval));
            memmove(ss2,s2,sizeof(Interval));
            spi = i;
        }
    }

    // merge split operation
    mxmsd = -10e12;
    for( i = 0; i < l - 1; i++){
        j = i + 1;
        memmove(t,M[i],sizeof(Interval));
        mergeInterval(t,M[j]);
        msd = getDelta(M[i],M[j],t,l,n,LogD);
        msd += splitInterval(t, s1, s2, I, l - 1, m, n, LogD);
        if (msd > mxmsd){
            mxmsd = msd;
            memmove(ms1,s1,sizeof(Interval));
            memmove(ms2,s2,sizeof(Interval));
            msi = i;
        }
    }

    // merge merge split operation
    mxmmsd = -10e12;
    for (i = 0; i< l - 2; i++){
        j = i + 1; k = i + 2;
        memmove(t,M[i],sizeof(Interval));
        mergeInterval(t,M[j]);
        mmsd = getDelta(M[i], M[j], t, l, n, LogD);
        memmove(tt,t,sizeof(Interval));
        mergeInterval(t,M[k]);
        mmsd += getDelta(tt, M[k], t, l - 1, n, LogD);
        mmsd += splitInterval(t,s1,s2,I,l - 2,m,n, LogD);
        if (mmsd > mxmmsd){
            mxmmsd = mmsd;
            memmove(mms1,s1,sizeof(Interval));
            memmove(mms2,s2,sizeof(Interval));
            mmsi = i;
        }
    }

    *ol = l;
    if (mxspd > 0.0 && mxspd >= mxmsd && mxspd >= mxmmsd){
        if (spi < l - 1){
            memmove(M[spi + 2], M[spi + 1], sizeof(Interval) * (l - spi - 1));
        }
        memmove(M[spi],ss1,sizeof(Interval));
        memmove(M[spi + 1],ss2,sizeof(Interval));
        *ol += 1;
        return mxspd;
    }
    else if (mxmsd > 0.0 && mxmsd >= mxspd && mxmsd >= mxmmsd){
        memmove(M[msi],ms1,sizeof(Interval));
        memmove(M[msi + 1],ms2,sizeof(Interval));
        return mxmsd;
    }
    else if (mxmmsd > 0.0 && mxmmsd >= mxspd && mxmmsd >= mxmsd){
        memmove(M[mmsi],mms1,sizeof(Interval));
        memmove(M[mmsi + 1], mms2, sizeof(Interval));
        if (mmsi < l - 3){
            memmove(M[mmsi + 2],M[mmsi + 3],sizeof(Interval) * (l - mmsi - 3));
        }
        *ol -= 1;
        return mxmmsd;
    }
    else{
        return -1.0;
    }
}

/* ----------------------------------------
 * brief :  init Log Dict for calculation
 * LogD  :  the Log Dict
 * int L :  Dict length to init
 * return : the LogD cache
 * ---------------------------------------- */
double * initLogD(int L){
    if (L < 2)
        L = 2;
    double * LogD = (double*) malloc(sizeof(double) * L);
    memset(LogD,0,sizeof(double) * L);
    LogD[0] = 0; LogD[1] = 0;
    for(int i = 2; i < L; i++){
        LogD[i] = LogD[i - 1] + log(i);
    }
    return LogD;
}

/* ----------------------------------------------
 * brief    : the interface for the modl algorithm
 * double v : the value array
 * int a    : the label array
 * int n    : the length of array
 * int *nd  : the length of split rule point 
 * return   : the split point array
 * ---------------------------------------------- */
double * modl(double v[], int a[], double * LogD, int n, int * nd){
    int m = 0, l = 0, c = 0, i = 0;

    double * rule = NULL;

    // greedInit Interval first
    Interval * I = greedInit(v, a, n, &m);

    // exMerge Intervals
    Interval * M = exMerge(I, m, n, &l, LogD);

    // iteration for c count to be optimized
    while(optimize(M, l, I, m, n, &l, LogD) > 0.0){c += 1;}

    // the whole array, one Interval
    if (l < 2){
        *nd = 0;
        goto ret;
    }

    // generate the partition rule array
    rule = (double*)malloc(sizeof(double) * (l - 1));
    *nd = l - 1;
    for(i = 0; i < l-1; i++){
        rule[i] = (v[M[i][1]] + v[M[i+1][0]]) / 2;
    }

ret:
    // free the Intervals used 
    free(I); I = NULL;
    free(M); M = NULL;

    // return the rule
    return rule;
}
