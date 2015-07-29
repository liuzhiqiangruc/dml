/* ========================================================
 *   Copyright (C) 2015 All rights reserved.
 *   
 *   filename : pattern_detection.c
 *   author   : ***
 *   date     : 2015-02-09
 *   info     : 
 * ======================================================== */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "hash_pattern.h"


void help(){
    fprintf(stderr, "pattern_detection usage:\n");
    fprintf(stderr, "   ./pattern_detection ts_file[string] n[int] m[int] l[int] k[int]\n");
    fprintf(stderr, "     ts_file   : time series file\n");
    fprintf(stderr, "     n         : row number of file\n");
    fprintf(stderr, "     m         : col number of file\n");
    fprintf(stderr, "     l         : pattern length to detection\n");
    fprintf(stderr, "     k         : num of hash functions[0~64]\n");

}
int main(int argc, char *argv[]){
    if (argc < 6){
        help();
        return -1;
    }
    char * filename = argv[1];
    int n = atoi(argv[2]);
    int m = atoi(argv[3]);
    int l = atoi(argv[4]);
    int k = atoi(argv[5]);
    if (k > 64) k = 64;

    double * d = (double*)malloc(sizeof(double) * n * m);
    memset(d, 0, sizeof(double) * n * m);

    FILE * f = fopen(filename, "r");
    for (int i = 0; i < n * m; i++){
        fscanf(f, "%lf", d + i);
    }
    fclose(f);
    int nlen = 0;
    int (*pat)[3] = get_hash_pattern(d, n, m, l, k, &nlen);

    for (int i = 0; i < nlen; i++){
        printf("%d\t%d\t%d\n", pat[i][0], pat[i][1], pat[i][2]);
    }
    free(pat); pat = NULL;

    return 0;
}
