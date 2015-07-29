/* ========================================================
 *   Copyright (C) 2015 All rights reserved.
 *
 *   filename : bayesian_changepoint.c
 *   author   : ***
 *   date     : 2015-02-12
 *   info     : find change point in ts increasingly
 *              using bayesian method
 * ======================================================== */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#define pi 3.14159265358979323846264338327950288

void mean_sd(double *x, int n, double *mean, double *sd) {
    double s = 0.0, ss = 0.0;
    for (int i = 0; i < n; i++) {
        s += x[i];
        ss += x[i] * x[i];
    }
    *mean = s / n;
    *sd = sqrt((ss - (*mean) * s) / (n - 1));
}

void z_norm(double *x, int n, double *nx) {
    double mean, sd;
    mean_sd(x, n, &mean, &sd);
    for (int i = 0; i < n; i++) {
        nx[i] = (x[i] - mean) / sd;
    }
}

void tabulate(int *x, int n, int *t1, int *t2, int *tot) {
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

void constant_hazard(double *nr, int n, double lambda) {
    for (int i = 0; i < n; ++i) {
        nr[i] = 1 / lambda;
    }
}

double studentpdf(double x, double mu, double var, double nu) {
    double c = exp(lgamma(nu / 2.0 + 0.5) - lgamma(nu / 2.0)) * pow(nu * var * (double) pi, -0.5);
    double p = c * pow((1.0 + (1.0 / (nu * var)) * (x - mu) * (x - mu)), -(nu + 1.0) / 2.0);
    return p;
}

void bayesian_changepoint(double *X, int T,int *points) {
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

