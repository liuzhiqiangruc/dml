/* ========================================================
 *   Copyright (C) 2015 All rights reserved.
 *   
 *   filename : fits.c
 *   author   : liuzhiqiang01@baidu.com
 *              lizeming@baidu.com
 *   date     : 2015-07-29
 *   info     : time series fit using newton methods
 * ======================================================== */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "fit_newton.h"

void help() {
    fprintf(stderr, "\nfits usage:\n");
    fprintf(stderr, "./fits -f <string> -s <int> -n <int> -l <float>         \n");
    fprintf(stderr, "       -f  ts_file              : one point per line       \n");
    fprintf(stderr, "       -s  methods              : 1, L1 norm fit           \n");
    fprintf(stderr, "                                : 2, L2 norm fit           \n");
    fprintf(stderr, "       -n  Iter Num             : Iteration for fit        \n");
    fprintf(stderr, "       -l  smooth coefficient   : More Bigger More Smoother\n");
}

int main(int argc, char *argv[]) {
    if ((argc & 1) == 0 || argc == 1) {
        fprintf(stderr, "command line not well formatted\n");
        help();
        return -1;
    }
    int i = 0;
    char *arg = NULL;
    char *filename = NULL;
    double weight = 0;
    int method = 1;
    int m = 5;
    int iter = 100;
    while (i < argc) {
        arg = argv[i];
        if (0 == strcmp(arg, "-f")) {
            filename = argv[++i];
        } else if (0 == strcmp(arg, "-l")) {
            weight = atof(argv[++i]);
        } else if (0 == strcmp(arg, "-s")) {
            method = atoi(argv[++i]);
        } else if (0 == strcmp(arg, "-n")) {
            iter = atoi(argv[++i]);
        }
        i += 1;
    }
    int n = 0;
    FILE *f = NULL;
    if (NULL == (f = fopen(filename, "r"))) {
        fprintf(stderr, "can not open file \"%s\"\n", filename);
        return -1;
    }
    double number = 0;
    while (fscanf(f, "%lf", &number) != EOF) {
        n += 1;
    }
    double *retx  = malloc(sizeof(double) * n);
    double *datax = malloc(sizeof(double) * n);
    rewind(f);
    n = 0;
    while (fscanf(f, "%lf", &number) != EOF) {
        datax[n] = number;
        n += 1;
    }
    fit_newton(n, datax, weight , method, m, iter, retx);
    if (method == 1) {
        for (i = 1; i < n; ++i) {
            retx[i] += retx[i - 1];
        }
    }
    printf("x = [");
    for (int i = 1; i <= n; i++) {
        printf("%d, ", i);
    }
    printf("]\n");
    printf("y = [");
    for (long i = 0; i < n; i++) {
        printf("%lf, ", datax[i]);
    }
    printf("]\n");
    printf("ys = [");
    for (long i = 0; i < n; i++) {
        printf("%lf, ", retx[i]);
    }

    printf("]\n");
    printf("import pylab as plt\n");
    printf("plt.plot(x, y, label='xy')\n");
    printf("plt.plot(x, ys, label='xys')\n");
    printf("plt.legend()\n");
    printf("plt.show()\n");

    free(datax);
    free(retx);
    return 0;
}
