/* ========================================================
*   Copyright (C) 2015 All rights reserved.
*
*   filename : lr.c
*   author   : lizeming@baidu.com
*   date     : 2015-08-01
*   info     : logistic regression binary
* ======================================================== */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "idmap.h"
#include "lr.h"

void help() {
    fprintf(stderr, "\nlr(logistic regression) usage:         \n");
    fprintf(stderr, "./lr -f <string> -a <double> -r <int>    \n");
    fprintf(stderr, "     -f  data input file                 \n");
    fprintf(stderr, "     -a  regularized paramenter          \n");
    fprintf(stderr, "     -r  1:L1 Norm; 2: L2 Norm           \n");
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
    double lambda = 0.0;
    int    method = 0;
    double *y;
    int *len;
    int tlen;
    double *val;
    int *valid;
    int valpos;
    double *retx;

    FILE *f = NULL;
    int r = 0, c = 0, id;
    int step;
    char buffer[1024], str[1024];
    char *bfpos; //buffer point
    char *tstr = NULL;  //tmp str
    double num;
    IdMap *im = idmap_create();

    while (i < argc) {
        arg = argv[i];
        if (0 == strcmp(arg, "-f")) {
            filename = argv[++i];
        }
        else if (0 == strcmp(arg, "-a")){
            lambda = atof(argv[++i]);
        }
        else if (0 == strcmp(arg, "-r")){
            method = atoi(arg[++i]);
        }
        i += 1;
    }
    if (NULL == (f = fopen(filename, "r"))) {
        fprintf(stderr, "can not open file \"%s\"\n", filename);
        return -1;
    }
    if (method != 1 && method != 2){
        fprintf(stderr, "method must be 1, or 2.\n");
        return -1;
    }

    //get col number
    r = tlen = 0;
    while (fgets(buffer, 1024, f) != NULL) {
        bfpos = buffer;
        while (*bfpos != '\t') {
            bfpos += 1;
        }
        bfpos += 1;
        while (*bfpos != '\0') {
            sscanf(bfpos, "%s", str);
            tlen += 1;
            step = 0;
            while (*bfpos != '\t') {
                bfpos += 1, step += 1;
            }
            bfpos += 1, step += 1;
            //insert str to idmap
            tstr = malloc(sizeof(char) * step);
            memcpy(tstr, str, step - 1);
            tstr[step - 1] = '\0';
            idmap_add(im, tstr, idmap_size(im));
            while (*bfpos != '\t' && *bfpos != '\0') {
                bfpos += 1;
            }
            if (*bfpos != '\0') bfpos += 1;
        }
        r += 1;
    }
    //build data
    y = (double *) malloc(sizeof(double) * r);
    len = (int *) malloc(sizeof(int) * r);
    rewind(f);
    c = idmap_size(im);
    val = (double *)malloc(sizeof(double) * tlen);
    valid = (int *)malloc(sizeof(int) * tlen);
    r = 0;
    valpos = 0;
    while (fgets(buffer, 1024, f) != NULL) {
        bfpos = buffer;
        sscanf(bfpos, "%lf", y + r);
        while (*bfpos != '\t') {
            bfpos += 1;
        }
        len[r] = 0;
        bfpos += 1;
        while (*bfpos != '\0') {
            sscanf(bfpos, "%s\t%lf", str, &num);
            len[r] += 1;
            while (*bfpos != '\t') {
                bfpos += 1;
            }
            bfpos += 1;
            id = idmap_get_value(im, str);
            val[valpos] = num;
            valid[valpos] = id;
            valpos++;
            while (*bfpos != '\t' && *bfpos != '\0') {
                bfpos += 1;
            }
            if (*bfpos != '\0') bfpos += 1;
        }
        r++;
    }
    retx = (double *) malloc(sizeof(double) * c);
    lr(r, c, tlen, len, valid, val, y, lambda, method, retx);
    for (idmap_reset(im), i = 0; i < c; ++i) {
        idmap_next(im, &tstr, NULL);
        printf("%s:", tstr);
        printf("%lf\n", retx[i]);
    }
    free(y);
    free(len);
    free(val);
    free(valid);
    free(retx);
    idmap_free(im);
    return 0;
}
