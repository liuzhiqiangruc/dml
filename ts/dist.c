/* ========================================================
 *   Copyright (C) 2014 All rights reserved.
 *   
 *   filename : dp.c
 *   author   : liuzhiqiang01@baidu.com
 *   date     : 2014-12-08
 *   info     : dtw and lcss
 * ======================================================== */

#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "dist.h"

#define max(a,b) (a)<(b)?(b):(a)
#define min(a,b) (a)<(b)?(a):(b)

double dtw(double * x, int nx, double * y, int ny){
    if (nx < 1 || ny < 1 || !x || !y)
        return -1.0;
    int i = 0, j = 0, t = 0;
    double mp1 = 0.0, mp2 = 0.0, mp3 = 0.0, mp = 0.0, dist = 0.0;
    double * dm = (double *)malloc(sizeof(double) * nx * ny);
    memset(dm, 0, sizeof(double) * nx * ny);
    for (i = 0; i < nx; i++){
        for (j = 0; j < ny; j++){
            dm[i * ny + j] = (x[i] - y[j]) * (x[i] - y[j]);
        }
    }
    for (i = 1; i < nx; i++){
        dm[i * ny] += dm[(i-1) * ny];
    }
    for (j = 1; j < ny; j++){
        dm[j] += dm[j-1];
    }
    for (i = 1; i < nx; i++){
        t = i * ny;
        for (j = 1; j < ny; j++){
            mp1 = dm[t - ny + j - 1] + dm[t + j];
            mp2 = dm[t - ny + j];
            mp3 = dm[t + j - 1];
            if (mp1 < mp2) mp = mp1 < mp3 ? mp1 : mp3;
            else mp = mp2 < mp3 ? mp2 : mp3;
            dm[t + j] += mp;
        }
    }
    dist = sqrt(dm[ny * nx - 1]);
    free(dm); dm = NULL;
    return dist;
}

int ed_bak(char * x, char * y){
    if (!x || !y)
        return 0;
    int dist = 0;
    int lx = strlen(x);
    int ly = strlen(y);
    if (lx < 1 || ly < 1)
        return 0;
    lx += 1;
    ly += 1;
    int * dm = (int*)malloc(sizeof(int) * lx * ly);
    memset(dm, 0, sizeof(int) * lx * ly);
    for (int i = 0; i < lx; i++){
        dm[i * ly] = i;
    }
    for (int i = 0; i < ly; i++){
        dm[i] = i;
    }
    for (int i = 1, offs = ly; i < lx; i++, offs += ly){
        for (int j = 1; j < ly; j++){
            int d = 1;
            if (x[i - 1] == y[j - 1]){
                d = 0;
            }
            int tmp = min(dm[offs - ly + j] + 1, dm[offs + j - 1] + 1);
            dm[offs + j] = min(tmp, dm[offs + j - 1 - ly] + d);
        }
    }
    //for (int i = 0; i < lx; i++){
    //    for (int j = 0; j < ly; j++){
    //        printf ("%3d", dm[i * ly + j]);
    //    }
    //    printf("\n");
    //}
    dist = dm[lx * ly - 1];
    free(dm);
    dm = NULL;
    return dist;
}

int ed(char * x, char * y){
    if (!x || !y)
        return 0;
    int dist = 0, t_dist = 0, tmp = 0, d = 0;
    int lx = strlen(x), ly = strlen(y);
    int * dm = NULL;
    char * t = NULL;
    if (lx < 1 || ly < 1)
        return 0;
    lx += 1;
    ly += 1;
    if (lx < ly){
        t = x; x = y; y = t;
        dist = lx; lx = ly; ly = dist;
    }
    dm = (int*)malloc(sizeof(int) * ly);
    memset(dm, 0, sizeof(int) * ly);
    for (int i = 0; i < ly; i++){
        dm[i] = i;
    }
    for (int i = 1; i < lx; i++){
        dist = i;
        t_dist = 0;
        for (int j = 1; j < ly; j++){
            d = 1;
            if (x[i - 1] == y[j - 1]){
                d = 0;
            }
            tmp = min(dm[j] + 1, dist + 1);
            t_dist = min(tmp, dm[j - 1] + d);
            dm[j - 1] = dist;
            dist = t_dist;
        }
        dm[ly - 1] = dist;
    }
    free(dm); dm = NULL;
    return dist;
}

int lcss(int * x, int nx, int *y, int ny){
    if (!x || !y || nx < 2 || ny < 2){
        return 0;
    }
    nx += 1; ny += 1;
    int k = 0, l = 0;
    int * matrix = (int*)malloc(sizeof(int) * nx * ny);
    memset(matrix, 0, sizeof(int) * nx * ny);
    for (int i = 1; i < nx; ++i){
        for (int j = 1; j < ny; ++j){
            k = i * ny + j;
            l = k - ny - 1;
            if (x[i - 1] == y[j - 1]){
                matrix[k] = matrix[l] + 1;
            }
            else{
                matrix[k] = max(matrix[l + 1], matrix[k - 1]);
            }
        }
    }
    l = matrix[nx * ny - 1];
    free(matrix); matrix = NULL;
    return l;
}
