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
    fprintf(stderr, "     -b  1:binary or else                \n");
    fprintf(stderr, "     -r  1:L1 Norm; 2: L2 Norm           \n");
}

int main(int argc, char *argv[]) {
    if ((argc & 1) == 0 || argc == 1) {
        fprintf(stderr, "command line not well formatted\n");
        help();
        return -1;
    }
    int i = 0;
    //arg parameter
    char *arg = NULL;
    char *filename = NULL;
    double lambda = 0.0;
    int method = 0;
    int binary = 0;

    //data to intial and read
    double *y;
    int *len;
    int totlen, tlen;
    double *val;
    int *valid;
    int valpos;
    double *retx;
    int r = 0, c = 0, id;

    //temp var
    FILE *f = NULL;
    int step;
    char buffer[1024], str[1024];
    char *bfpos;
    char *tstr = NULL;
    double num;
    while (i < argc) {
        arg = argv[i];
        if (0 == strcmp(arg, "-f")) {
            filename = argv[++i];
        }
        else if (0 == strcmp(arg, "-a")) {
            lambda = atof(argv[++i]);
        }
        else if (0 == strcmp(arg, "-r")) {
            method = atoi(argv[++i]);
        }
        else if (0 == strcmp(arg, "-b")) {
            binary = atoi(argv[++i]);
        }
        i += 1;
    }
    if (NULL == (f = fopen(filename, "r"))) {
        fprintf(stderr, "can not open file \"%s\"\n", filename);
        return -1;
    }
    if (method != 1 && method != 2) {
        fprintf(stderr, "method must be 1, or 2.\n");
        return -1;
    }
    //get row number and total len;
    r = totlen = 0;
    while (fgets(buffer, 1024, f) != NULL) {
        bfpos = buffer;
        while (*bfpos != '\t') ++bfpos;
        sscanf(bfpos, "\t%d", &tlen);
        totlen += tlen;
        ++r;
    }
    //initial memory
    val = binary ? NULL : (double *) malloc(sizeof(double) * totlen);
    y = (double *) malloc(sizeof(double) * r);
    len = (int *) malloc(sizeof(int) * r);
    valid = (int *) malloc(sizeof(int) * totlen);
    IdMap *im = idmap_create();
    r = valpos = 0;
    rewind(f);
    //build data
    while (fgets(buffer, 1024, f) != NULL) {
        bfpos = buffer;
        sscanf(bfpos, "%lf", y + r);
        while (*bfpos != '\t') ++bfpos;
        ++bfpos;
        sscanf(bfpos, "%d", len + r);
        while (*bfpos != '\t') ++bfpos;
        ++bfpos;
        for (i = 0; i < len[r]; ++i) {
            sscanf(bfpos, "%s", str);
            step = 0;
            while (*bfpos != '\t') ++bfpos, ++step;
            ++bfpos, ++step;
            tstr = malloc(sizeof(char) * step);
            memcpy(tstr, str, step), tstr[step - 1] = '\0';
            if (!binary) {
                sscanf(bfpos, "%lf", &num);
                while (*bfpos != '\t') ++bfpos;
                ++bfpos;
                val[valpos] = num;
            }
            id = idmap_get_value(im, tstr);
            if (id == -1) {
                id = idmap_size(im);
                idmap_add(im, tstr, idmap_size(im));
            }
            valid[valpos] = id;
            valpos++;
        }
        ++r;
    }
    c = idmap_size(im);
    retx = (double *) malloc(sizeof(double) * c);
    lr(r, c, totlen, len, valid, val, y, lambda, method, retx);
    for (idmap_reset(im), i = 0; i < c; ++i) {
        idmap_next(im, &tstr, &id);
        printf("%s: ", tstr);
        printf("%.10lf\n", retx[id]);
    }
    free(y);
    free(len);
    free(val);
    free(valid);
    free(retx);
    idmap_free(im);
    return 0;
}
