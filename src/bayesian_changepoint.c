/* ========================================================
 *   Copyright (C) 2015 All rights reserved.
 *
 *   filename : pattern_detection.c
 *   author   : ***
 *   date     : 2015-02-11
 *   info     :
 * ======================================================== */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "bayesian_changepoint.h"

void help() {
    fprintf(stderr, "bayesian_changepoint usage:\n");
    fprintf(stderr, "   ./bayesian_changepoint ts_file[string] n[int]\n");
    fprintf(stderr, "     ts_file   : time series file\n");
    fprintf(stderr, "     n         : row number of file\n");
}

int main(int argc, char *argv[]) {
    if (argc < 3) {
        help();
        return -1;
    }
    char *filename = argv[1];
    FILE *f = fopen(filename, "r");

    int T = atoi(argv[2]);
    double *data = malloc(sizeof(double) * T);
    int *points = malloc(sizeof(int) * (T + 1));
    
    for (int i = 0; i < T; ++i) {
        fscanf(f, "%lf", data + i);
    }
    fclose(f);
    
    bayesian_changepoint(data, T, points);
    for (int i = 2; i < T; ++i) {
        if (points[i] > 0){
            printf("%d\n", i);
        }
    }
    free(data);
    free(points);
    return 0;
}
