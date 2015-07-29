/* ========================================================
 *   Copyright (C) 2014 All rights reserved.
 *   
 *   filename : edm.c
 *   author   : liuzhiqiang01@baidu.com
 *   date     : 2014-12-24
 *   info     : implement for edm breakout detection
 * ======================================================== */


#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "mtrace.h"
#include "fn_type.h"
#include "edm.h"

static double Linear(double x){
    return 1;
}
static double Const(double x){
    return 0;
}
static double Quadratic(double x){
    return 2 * x + 1;
}

int cmp_fn(double *m, double *n){
    double l = *m - *n;
    return l > 0 ? 1 : (l < 0 ? -1 : 0);
}

int * breakout(double * ts, int n, int min_size, double beta, int degree, int *ol){

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

