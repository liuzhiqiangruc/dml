/* ========================================================
 *   Copyright (C) 2016 All rights reserved.
 *   
 *   filename : breakout.c
 *   author   : ***
 *   date     : 2016-01-15
 *   info     : 
 * ======================================================== */
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include "median.h"
#include "breakout.h"

#define pi 3.14159265358979323846264338327950288


static double Linear(double x){
    return 1;
}
static double Const(double x){
    return 0;
}
static double Quadratic(double x){
    return 2 * x + 1;
}

static int cmp_fn(double *m, double *n){
    double l = *m - *n;
    return l > 0 ? 1 : (l < 0 ? -1 : 0);
}

static void lfit(double *s, int n, double * l0, double * l1){
    int i;
    double t;
    double g0   = 0.0, g1   = 0.0; 
    double h00  = 0.0, h01  = 0.0, h10  = 0.0, h11  = 0.0;
    double hv00 = 0.0, hv01 = 0.0, hv10 = 0.0, hv11 = 0.0;
    // grad and hessian matrix
    for (i = 0; i < n; i++){
        t    = s[i];
        g0  += t;
        g1  += t * (1.0 + i);
        h00 += 1.0;
        h01 += 1.0 + i;
        h11 += (1.0 + i) * (1.0 + i);
    }
    h10 = h01;
    // inverse of hessian
    t = h00 * h11 - h01 * h10;
    hv00 = h11 / t;
    hv01 = hv10 = -h01 / t;
    hv11 = h00 /t;
    // the theta
    *l0 = hv00 * g0 + hv01 * g1;
    *l1 = hv10 * g0 + hv11 * g1;
}

static double loss(double * s, int n){
    int i; 
    double t, l0, l1;
    // learn l0, l1
    lfit(s, n, &l0, &l1);
    // sqare loss
    t = 0.0;
    for (i = 0; i < n; i++){
        t += (l0 + l1 * (i + 1) - s[i]) * (l0 + l1 * (i + 1) - s[i]);
    }
    return t;
}

static void rets(double *ts, int n, int *re, int k, double *rety){
    int b, e, i, j;
    double l0, l1, t;
    b = 0;
    for (i = 0; i < k; i++){
        e = re[i];
        lfit(ts + b, e - b, &l0, &l1);
        for (j = b; j < e; j++){
            t = l0 + l1 * (j - b + 1);
            rety[j] = t;
        }
        b = e;
    }
    e = n;
    lfit(ts + b, e - b, &l0, &l1);
    for (j = b; j < e; j++){
        t = l0 + l1 * (j - b + 1);
        rety[j] = t;
    }
}

static void mean_sd(double *x, int n, double *mean, double *sd) {
    double s = 0.0, ss = 0.0;
    for (int i = 0; i < n; i++) {
        s += x[i];
        ss += x[i] * x[i];
    }
    *mean = s / n;
    *sd = sqrt((ss - (*mean) * s) / (n - 1));
}

static void z_norm(double *x, int n, double *nx) {
    double mean, sd;
    mean_sd(x, n, &mean, &sd);
    for (int i = 0; i < n; i++) {
        nx[i] = (x[i] - mean) / sd;
    }
}

static void tabulate(int *x, int n, int *t1, int *t2, int *tot) {
    if (n == 0) return;
    *tot = 0;
    for (int i = 0; i < n; ++i) {
        int isAppear = 0;
        for (int j = 0; j < *tot; ++j) {
            if (x[i] != t1[j]) continue;
            t2[j] += 1;
            isAppear = 1;
            break;
        }
        if (isAppear) continue;
        t1[*tot] = x[i];
        t2[*tot] = 1;
        *tot += 1;
    }
}

static void constant_hazard(double *nr, int n, double lambda) {
    for (int i = 0; i < n; ++i) {
        nr[i] = 1 / lambda;
    }
}

static double studentpdf(double x, double mu, double var, double nu) {
    double c = exp(lgamma(nu / 2.0 + 0.5) - lgamma(nu / 2.0)) * pow(nu * var * (double) pi, -0.5);
    double p = c * pow((1.0 + (1.0 / (nu * var)) * (x - mu) * (x - mu)), -(nu + 1.0) / 2.0);
    return p;
}

int * edm(double * ts, int n, int min_size, double beta, int degree, int *ol){
    if (!ts || min_size < 5 || n < 2 * min_size || !ol){
        return NULL;
    }
    double (*G)(double);
    switch(degree){
        case 1 :  G = Linear;
                break;
        case 2 :  G = Quadratic;
                break;
        default : G = Const;
                break;
    }
    int * prev = (int*)malloc(sizeof(int) * (n + 1));
    memset(prev, 0, sizeof(int) * (n + 1));
    int * num  = (int*)malloc(sizeof(int) * (n + 1));
    memset(num, 0, sizeof(int) * (n + 1));
    double * F = (double*)malloc(sizeof(double) * (n + 1));
    memset(F, 0, sizeof(double) * (n + 1));
    MTrace * m1 = m_create((CMP_FN)cmp_fn, NULL);
    MTrace * m2 = m_create((CMP_FN)cmp_fn, NULL);
    for (int s = 2 * min_size; s < n + 1; ++s){
        m_clear(m1); m_clear(m2);
        for (int i = 0; i < min_size - 1; ++i){
            m_add(m1, ts + i);
        }
        for (int i = min_size - 1; i < s; ++i){
            m_add(m2, ts + i);
        }
        for (int t = min_size; t < s - min_size + 1; ++t){
            m_add(m1, ts + t - 1);
            m_remove(m2, ts + t - 1);
            if (prev[t] > prev[t - 1]){
                for (int i = prev[t - 1]; i < prev[t]; ++i){
                    m_remove(m1, ts + i);
                }
            }
            if (prev[t] < prev[t - 1]){
                for (int i = prev[t]; i < prev[t - 1]; ++i){
                    m_add(m1, ts + i);
                }
            }
            double lm = *(double*)get_median(m1);
            double rm = *(double*)get_median(m2);
            double normalize = ((t - prev[t]) * (s - t))       \
                             / ((double)(s - prev[t]) * (s - prev[t]));
            double tmp_s = F[t] + normalize * (lm - rm) * (lm - rm) - beta * G(num[t]);
            if (tmp_s > F[s]){
                num[s] = num[t] + 1;
                F[s] = tmp_s;
                prev[s] = t;
            }
        }
    }
    int k = num[n];
    *ol = k;
    int * re = (int*)malloc(sizeof(int) * k);
    memset(re, 0, sizeof(int) * k);
    int i = n;
    while(i > 0){
        if (prev[i])
            re[--k] = prev[i];
        i = prev[i];
    }
    free(prev); prev = NULL;
    free(num);  num  = NULL;
    free(F);    F    = NULL;
    m_free(m1); m1   = NULL;
    m_free(m2); m2   = NULL;
    return re;
}

int * lsp(double * ts, int n, int min_size, double beta, int *ol, double * rety){
    if (!ts || min_size < 2 || n < 2 * min_size || !ol){
        return NULL;
    }
    // prev breakout point
    int * prev = (int*)malloc(sizeof(int) * (n + 1));
    memset(prev, 0, sizeof(int) * (n + 1));
    // number of breakout point
    int * num  = (int*)malloc(sizeof(int) * (n + 1));
    memset(num, 0, sizeof(int) * (n + 1));
    // F scores
    double * F = (double*)malloc(sizeof(double) * (n + 1));
    memset(F, 0, sizeof(double) * (n + 1));
    for (int s = 2 * min_size; s < n + 1; ++s){
        for (int t = min_size; t < s - min_size + 1; ++t){
            double ls = loss(ts + prev[t], t - prev[t]);
            double rs = loss(ts + t, s - t);
            double as = loss(ts + prev[t], s - prev[t]);
            double score = (as - ls - rs) * (t - prev[t]) * (s - t) /    \
                           ((s - prev[t]) * (s - prev[t])) - num[t] * beta;
            score += F[t];
            if (score > F[s]){
                num[s] = num[t] + 1;
                F[s] = score;
                prev[s] = t;
            }
        }
    }
    int k = num[n];
    *ol = k;
    int *re = NULL;
    if (k > 0) {
        re = (int*)malloc(sizeof(int) * k);
        memset(re, 0, sizeof(int) * k);
        int i = n;
        while(i > 0){
            if (prev[i])
                re[--k] = prev[i];
            i = prev[i];
        }
    }
    if (rety){
        rets(ts, n, re, *ol, rety);
    }
    free(prev);  prev  = NULL;
    free(num);   num   = NULL;
    free(F);     F     = NULL;
    return re;
}

void bcp(double *X, int T,int *points) {
    double lambda = 500;
    int mu0 = 0;
    int kappa0 = 1;
    int alpha0 = 1;
    int beta0 = 4;
    
    z_norm(X, T, X);
    
    double (*R)[2] = (double (*)[2]) malloc(sizeof(double) * 2 * (T + 1));
    memset(R, 0, sizeof(double) * 2 * (T + 1));
    R[0][0] = 1;

    double *muT = malloc(sizeof(double) * (T + 1));
    double *kappaT = malloc(sizeof(double) * (T + 1));
    double *alphaT = malloc(sizeof(double) * (T + 1));
    double *betaT = malloc(sizeof(double) * (T + 1));
    muT[T] = mu0;
    kappaT[T] = kappa0;
    alphaT[T] = alpha0;
    betaT[T] = beta0;

    // Keep track of the maximums.
    int *maxes = malloc(sizeof(int) * (T + 1));
    memset(maxes, 0, sizeof(int) * (T + 1));
    int win_size = 100;
    memset(points, 0, sizeof(int) * T);


    int *rst = malloc(sizeof(int) * T);
    memset(rst, 0, sizeof(int) * T);

    // Loop over the data like we're seeing it all for the first time.
    double *predprobs = malloc(sizeof(double) * (T + 1));
    double *H = malloc(sizeof(double) * T);

    int *table1 = malloc(sizeof(int) * win_size);
    int *table2 = malloc(sizeof(int) * win_size);
    for (int t = 0; t < T; ++t) {

        /* Evaluate the predictive distribution for the new datum under each of
         the parameters.  This is the standard thing from Bayesian inference.
         */
        for (int i = T - t; i <= T; ++i) {
            predprobs[i] = studentpdf(X[t], muT[i], betaT[i] * (kappaT[i] + 1.0) / (alphaT[i] * kappaT[i]), 2.0 * alphaT[i]);
        }

        // Evaluate the hazard function for this interval.
        constant_hazard(H, t + 1, lambda);

        /*Evaluate the growth probabilities - shift the probabilities down
        and to the right, scaled by the hazard function and the predictive probabilities.
         */

        for (int i = 0; i <= t; ++i) {
            R[i + 1][1] = R[i][0] * predprobs[T - t + i] * (1.0 - H[i]);
        }

        R[0][1] = 0;
        for (int i = 0; i <= t; ++i) {
            R[0][1] += R[i][0] * predprobs[T - t + i] * H[i];
        }

        double sum = 0;
        for (int i = 0; i <= t + 1; ++i) {
            sum += R[i][1];
        }
        for (int i = 0; i <= t + 1; ++i) {
            R[i][1] = R[i][1] / sum;
        }

        // Update the parameter sets for each possible run length.
        betaT[T - t - 1] = beta0;
        for (int i = T - t; i <= T; ++i) {
            betaT[i] = betaT[i] + (kappaT[i] * (X[t] - muT[i]) * (X[t] - muT[i]) / (2.0 * (kappaT[i] + 1.0)));
        }
        muT[T - t - 1] = mu0;
        for (int i = T - t; i <= T; ++i) {
            muT[i] = (kappaT[i] * muT[i] + X[t]) / (kappaT[i] + 1);
        }
        kappaT[T - t - 1] = kappa0;
        for (int i = T - t; i <= T; ++i) {
            kappaT[i] += 1.0;
        }
        alphaT[T - t - 1] = alpha0;
        for (int i = T - t; i <= T; ++i) {
            alphaT[i] += 0.5;
        }

        int mxid = 0;
        for (int i = 1; i <= t + 1; ++i) {
            if (R[mxid][0] < R[i][0]) mxid = i;
        }
        maxes[t] = mxid;

        for (int i = 0; i <= t + 1; ++i) {
            R[i][0] = R[i][1];
        }

        rst[t] = t - maxes[t] + 1;

        if (t + 1 >= win_size) {
            int *candidates = rst + (t + 1 - win_size);

            int tot = 0;
            tabulate(candidates, win_size, table1, table2, &tot);

            int F = 0; //max value
            int TT = 0; //max value's index
            for (int i = 1; i < tot; ++i) {
                if (table2[i] > table2[TT]) TT = i;
            }
            F = table2[TT];
            for (int i = 0; i < tot; ++i) {
                if (table2[i] == F) {
                    points[table1[i]] += 1;
                }
            }
        }
    }
    free(R);
    free(muT);
    free(kappaT);
    free(alphaT);
    free(betaT);
    free(maxes);
    free(rst);
    free(predprobs);
    free(H);
    free(table1);
    free(table2);
}
