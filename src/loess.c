/* ========================================================
*   Copyright (C) 2015 All rights reserved.
*
*   filename : loess.c
*   author   : ***
*   date     : 2015-04-03
*   info     :
* ======================================================== */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "loess.h"


void help() {
    fprintf(stderr, "loess usage:\n");
    fprintf(stderr, "   ./loess -f[string] -d[int] -robust[int]\n\n");
    fprintf(stderr, "     -f(ts_file)   : data file which contain the pair of x and y\n");
    fprintf(stderr, "     -d(dist)  : point within d will be concerned,other wise will take all point, at least 1, default is 1\n");
    fprintf(stderr, "     -robust   : robust iter\n");
}

int main(int argc, char *argv[]) {
    if ((argc & 1) == 0 || argc == 1) {
        fprintf(stderr, "command line not well formatted\n");
        help();
        return -1;
    }

    char *arg = NULL;
    char *input = NULL;
    int i = 0;
    int robust = 0;
    int limitDist = 1;
    while (i < argc) {
        arg = argv[i];
        if (0 == strcmp(arg, "-f")) {
            input = argv[++i];
        } else if (0 == strcmp(arg, "-d")) {
            limitDist = atoi(argv[++i]);
            if (limitDist < 6) {
                fprintf(stderr, "-d dist must not less than 6 \n");
                return -1;
            }
        } else if (0 == strcmp(arg, "-robust")) {
            robust = atoi(argv[++i]);
        }
        i += 1;
    }

    int totPoint = 0;
    FILE *f = NULL;
    if (NULL == (f = fopen(input, "r"))) {
        fprintf(stderr, "can not open file \"%s\"\n", input);
        return -1;
    }
    double number = 0;
    while (fscanf(f, "%lf", &number) != EOF) {
        totPoint++;
    }
    double *datax = (double*)malloc(sizeof(double) * totPoint);
    rewind(f);
    totPoint = 0;
    while (fscanf(f, "%lf", &number) != EOF) {
        datax[totPoint] = number;
        totPoint += 1;
    }
    double *rety = (double*)malloc(sizeof(double) * totPoint);
    loess(datax, totPoint, limitDist, robust, rety);

    for (int i = 0; i < totPoint; i++) {
        printf("%lf\n", rety[i]);
    }

    free(datax);
    free(rety);

    return 0;
}
