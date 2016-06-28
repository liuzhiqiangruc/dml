/* ========================================================
 *   Copyright (C) 2016 All rights reserved.
 *   
 *   filename : k2d.c
 *   author   : liuzhiqiangruc@126.com
 *   date     : 2016-06-28
 *   info     : 
 * ======================================================== */
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "k2d.h"

static void update_center(double (*c)[11], int * ks, int k){
    int i;
    double mx, my, xx, yy, xy, det, invxx, invyy, invxy;
    for (i = 0; i < k; i++){
        mx = c[i][0] / ks[i];
        my = c[i][1] / ks[i];
        xx = c[i][2] / ks[i] - mx * mx;
        yy = c[i][3] / ks[i] - my * my;
        xy = c[i][4] / ks[i] - mx * my;
        det = xx * yy - xy * xy;
        invxx = yy / det;
        invyy = xx / det;
        invxy = -xy / det;
        c[i][5] = mx;
        c[i][6] = my;
        c[i][7] = det;
        c[i][8] = invxx;
        c[i][9] = invyy;
        c[i][10] = invxy;
    }
}

static void init_centers(double (*x)[2], double (*c)[11], int *ks, int *assign, int n, int k){
    int i, d;
    memset(c, 0, sizeof(double[11]) * k);
    memset(ks, 0, sizeof(int) * k);
    for (i = 0; i < n; i++){
        d = (int)(rand() + 0.1) / (RAND_MAX + 0.1) * k;
        printf("%d\n", d);
        c[d][0] += x[i][0];
        c[d][1] += x[i][1];
        c[d][2] += x[i][0] * x[i][0];
        c[d][3] += x[i][1] * x[i][1];
        c[d][4] += x[i][0] * x[i][1];
        ks[d]   += 1;
        assign[i] = d;
    }
    update_center(c, ks, k);
}

double (*k2d(double (*x)[2], int *assign, int n, int k))[6] {
    double (*centers)[11] = (double (*)[11])malloc(sizeof(double[11]) * k);
    double (*cls)[6] = (double (*)[6])malloc(sizeof(double[6]) * k);
    double dx, dy, det, invxx, invyy, invxy, pr, mxpr;
    int *ks = (int*)malloc(sizeof(int) * k);
    int i, j, t, mi;
    init_centers(x, centers, ks, assign, n, k);
    do {
        t = 0;
        for (i = 0; i < n; i++){
            mxpr = -1.0;
            mi = -1;
            for (j = 0; j < k; j++){
                dx = x[i][0] - centers[j][5];
                dy = x[i][1] - centers[j][6];
                det = centers[j][7];
                invxx = centers[j][8];
                invyy = centers[j][9];
                invxy = centers[j][10];
                pr = 1.0 / (sqrt(det) * exp(0.5 * (dx * dx * invxx + dy * dy * invyy + 2.0 * dx * dy * invxy)));
                if (pr > mxpr){
                    mxpr = pr;
                    mi = j;
                }
            }
            if (assign[i] != mi){
                t += 1;
                ks[assign[i]] -= 1;
                centers[assign[i]][0] -= x[i][0];
                centers[assign[i]][1] -= x[i][1];
                centers[assign[i]][2] -= x[i][0] * x[i][0];
                centers[assign[i]][3] -= x[i][1] * x[i][1];
                centers[assign[i]][4] -= x[i][0] * x[i][1];
                assign[i] = mi;
                ks[assign[i]] += 1;
                centers[assign[i]][0] += x[i][0];
                centers[assign[i]][1] += x[i][1];
                centers[assign[i]][2] += x[i][0] * x[i][0];
                centers[assign[i]][3] += x[i][1] * x[i][1];
                centers[assign[i]][4] += x[i][0] * x[i][1];
            }
        }
        update_center(centers, ks, k);
    } while (t < (n >> 10));
    for (i = 0; i < k; i++){
        cls[i][0] = centers[i][5];
        cls[i][1] = centers[i][6];
        cls[i][2] = centers[i][2] / ks[i] - centers[i][5] * centers[i][5];
        cls[i][3] = centers[i][3] / ks[i] - centers[i][6] * centers[i][6];
        cls[i][4] = centers[i][4] / ks[i] - centers[i][5] * centers[i][6];
        cls[i][5] = centers[i][7];
    }
    free(centers); centers = NULL;
    free(ks);      ks = NULL;
    return cls;
}
