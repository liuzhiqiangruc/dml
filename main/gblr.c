/* ========================================================
 *   Copyright (C) 2016 All rights reserved.
 *   
 *   filename : gblr.c
 *   author   : liuzhiqiangruc@126.com
 *   date     : 2016-03-28
 *   info     : 
 * ======================================================== */
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "gblr.h"

void help() {
    fprintf(stderr, "\ngblr usage:        \n");
    fprintf(stderr, "\n./gblr -n <int> -m <int> -d <int> -b <int> -a <double> -g <double> -r <double> -f <string> -t <string> -o <string>\n");
    fprintf(stderr, "     -n  tree capicity                   \n");
    fprintf(stderr, "     -m  max leaf node in per tree       \n");
    fprintf(stderr, "     -d  max depth of trees              \n");
    fprintf(stderr, "     -b  1:binary or else                \n");
    fprintf(stderr, "     -a  node regulization               \n");
    fprintf(stderr, "     -g  weight regulization             \n");
    fprintf(stderr, "     -r  learning rate                   \n");
    fprintf(stderr, "     -f  train input file                \n");
    fprintf(stderr, "     -t  test  input file                \n");
    fprintf(stderr, "     -o  otuput dir                      \n");
}

int parse_command_line(GBMP *p, int argc, char *argv[]){
    double r = 0.0;
    double w_reg = 0.0;
    double n_reg = 0.0;
    int b = 0, n = 20, m = 10, d = 2;
    char * f = NULL;
    char * t = NULL;
    char * o = "./";
    int i = 0;
    char * arg = NULL;
    if ((argc & 1) == 0){
        fprintf(stderr, "command line not well formatted\n");
        return -1;
    }
    while (i < argc) {
        arg = argv[i];
        if (0 == strcmp(arg,"-n")){
            n = atoi(argv[++i]);
        }
        else if (0 == strcmp(arg,"-m")){
            m = atoi(argv[++i]);
        }
        else if (0 == strcmp(arg,"-b")){
            b = atoi(argv[++i]);
        }
        else if (0 == strcmp(arg,"-d")){
            d = atoi(argv[++i]);
        }
        else if (0 == strcmp(arg,"-a")){
            n_reg = atof(argv[++i]);
        }
        else if (0 == strcmp(arg,"-g")){
            w_reg = atof(argv[++i]);
        }
        else if (0 == strcmp(arg,"-r")){
            r = atof(argv[++i]);
        }
        else if (0 == strcmp(arg,"-f")){
            f = argv[++i];
        }
        else if (0 == strcmp(arg,"-t")){
            t = argv[++i];
        }
        else if (0 == strcmp(arg,"-o")){
            o = argv[++i];
        }
        i += 1;
    }
    if (NULL == f) {
        fprintf(stderr,"no input data exit(-1)\n");
        return -1;
    }
    if (b != 0 && b != 1){
        fprintf(stderr, "binary must be 0, or 1\n");
        return -1;
    }
    p->max_trees      = n;
    p->max_leaf_nodes = m;
    p->binary         = b;
    p->max_depth      = d;
    p->nod_reg        = n_reg;
    p->wei_reg        = w_reg;
    p->rate           = r;
    p->train_input    = f;
    p->test_input     = t;
    p->out_dir        = o;
    return 0;
}

int main(int argc, char *argv[]) {
    GBMP p;
    if (-1 == parse_command_line(&p, argc, argv)){
        help();
        return -1;
    }
    fprintf(stderr, "command line parse done\n");
    GBDT * gblr = gbdt_lr(p);
    if (!gblr){
        return -1;
    }
    long t1 = time(NULL);
    gbdt_train(gblr);
    long t2 = time(NULL);
    printf("time used for training: %ld seconds\n", t2 - t1);
    gbdt_save(gblr);
    gbdt_free(gblr);
    gblr = NULL;
    return 0;
}
