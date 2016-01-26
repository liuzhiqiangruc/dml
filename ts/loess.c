/* ========================================================
 *   Copyright (C) 2015 All rights reserved.
 *
 *   filename : loess.c
 *   author   : lizeming@baidu.com
 *              liuzhiqiang@baidu.com
 *   date     : 2015-04-17
 *   info     : local weighted 2-d regression implementation
 * ======================================================== */

#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "median.h"
#include "loess.h"

int cmp_fn(double *m, double *n) {
    double l = *m - *n;
    return l > 0 ? 1 : (l < 0 ? -1 : 0);
}

static void grad_fn (double *y, double *w, int k, double *grad) {
    double t = 0.0;
    memset(grad, 0, sizeof(double) * 3);
    for (int i = 0; i < k; i++){
        t = w[i] * y[i];
        grad[0] += t;
        grad[1] += t * (1.0 + i);
        grad[2] += t * (1.0 + i) * (1.0 + i);
    }
}

static void hessian_v(double *w, double (*hv)[3], int k) {
    double div = 0.0;
    double h[3][3];
    memset(h, 0, sizeof(double) * 9);
    for (int i = 0; i < k; i++){
        h[0][0] += w[i];    
        h[0][1] += w[i] * (1.0 + i);  
        h[1][1] += w[i] * (1.0 + i) * (1.0 + i);
        h[1][2] += w[i] * (1.0 + i) * (1.0 + i) * (1.0 + i);
        h[2][2] += w[i] * (1.0 + i) * (1.0 + i) * (1.0 + i) * (1.0 + i);
        h[2][0]  = h[0][2]  = h[1][1];
        h[1][0]  = h[0][1];
        h[2][1]  = h[1][2];
    }

    div = h[0][0] * h[1][1] * h[2][2] - h[0][0] * h[1][2] * h[2][1] - \
          h[0][1] * h[1][0] * h[2][2] + h[0][1] * h[1][2] * h[2][0] + \
          h[0][2] * h[1][0] * h[2][1] - h[0][2] * h[1][1] * h[2][0];
    hv[0][0] = (h[1][1] * h[2][2] - h[1][2] * h[2][1]) / div;
    hv[1][1] = (h[0][0] * h[2][2] - h[0][2] * h[2][0]) / div;
    hv[2][2] = (h[0][0] * h[1][1] - h[0][1] * h[1][0]) / div;
    hv[0][1] = hv[1][0] = (h[0][2] * h[2][1] - h[0][1] * h[2][2]) / div;
    hv[0][2] = hv[2][0] = (h[1][0] * h[2][1] - h[1][1] * h[2][0]) / div;
    hv[1][2] = hv[2][1] = (h[0][1] * h[2][0] - h[0][0] * h[2][1]) / div;
}

static void pred(double *g, double (*hv)[3], int id, double *rety){
    double l1, l2, l3;
    l1 = hv[0][0] * g[0] + hv[0][1] * g[1] + hv[0][2] * g[2];
    l2 = hv[1][0] * g[0] + hv[1][1] * g[1] + hv[1][2] * g[2];
    l3 = hv[2][0] * g[0] + hv[2][1] * g[1] + hv[2][2] * g[2];
    *rety = l1 + l2 * (1.0 + id) + l3 * (1.0 + id) * (1.0 + id);
}

void w_regr(double *y, double *w, int s, int id, double * rv) {
    double g[3];
    double hv[3][3];
    hessian_v(w, hv, s);
    grad_fn(y, w, s, g);
    pred(g, hv, id, rv);
}

void loess(double *datax, int n, int span, int robust, double *rety) {
    if (!datax || !rety) {
        return;
    }
    if (span > n) {
        span = n;
    }
    if (span < 6){
        memcpy(rety, datax, sizeof(double) * n);
        return;
    }

    int    st = 0;
    double *y = datax;
    double g[3];         
    double hv[3][3];    

    double *w= (double *)malloc(sizeof(double) * span);
    memset(w, 0, sizeof(double) * span);

    for (int i = 0; i < n; i++) {
        if (i > (span >> 1) && (i - (span >> 1) + span - 1) < n) {
            y += 1;
            st += 1;
        }
        else {
            int l_span = i - st;
            int r_span = st + span - i - 1;
            int mxd = l_span < r_span ? r_span : l_span;
            int stt = st;
            for (int j = 0; j < span; j++) {
                w[j] = pow(1.0 - pow(1.0 * abs(i - stt) / mxd, 3), 3);
                stt += 1;
            }
            hessian_v(w, hv, span);
        }
        grad_fn(y, w, span, g);
        pred(g, hv, i - st, rety + i);
    }
    if (robust > 0) {
        MTrace *mtrace = m_create((CMP_FN) cmp_fn, NULL);
        double *rw = (double*)malloc(sizeof(double) * span);
        memset(rw, 0, sizeof(double) * span);
        double *res = (double*)malloc(sizeof(double) * n);
        memset(res, 0, sizeof(double) * n);

        for (int k = 0; k < robust; k++) {
            m_clear(mtrace);
            for (int i = 0; i < n; i++) {
                res[i] = fabs(datax[i] - rety[i]);
            }
            for (int i = 0; i < span; i++) {
                m_add(mtrace, res + i);
            }
            double median = *(double*)get_median(mtrace);
            double sixMedian = 6.0 * median;
            memset(w, 0, sizeof(double) * span);
            y = datax;
            st = 0;
            for (int i = 0; i < n; i++) {
                if (i > (span >> 1) && (i - (span >> 1) + span - 1) < n) {
                    m_remove(mtrace, res + st);
                    m_add(mtrace, res + st + span);
                    median = *(double*)get_median(mtrace);
                    sixMedian = 6.0 * median;
                    y += 1;
                    st += 1;
                }
                else {
                    int l_span = i - st;
                    int r_span = st + span - i - 1;
                    int mxd = l_span < r_span ? r_span : l_span;
                    int stt = st;
                    for (int j = 0; j < span; j++) {
                        w[j] = pow(1.0 - pow(1.0 * abs(i - stt) / mxd, 3), 3);
                        stt += 1;
                    }
                }
                int stt = st;
                for (int j = 0; j < span; j++) {
                    if (res[stt] < sixMedian) {
                        rw[j] = w[j] * pow(1.0 - pow(res[stt] / sixMedian, 2), 2);
                    }
                    else {
                        rw[j] = 0.0;
                    }
                    stt += 1;
                }
                w_regr(y, rw, span, i - st, rety + i);
            }
        }
        free(rw);  rw  = NULL;
        free(res); res = NULL;
        m_free(mtrace); mtrace = NULL;
    }
    free(w); w = NULL;
} 

