/* ========================================================
 *   Copyright (C) 2016 All rights reserved.
 *   
 *   filename : gblr.c
 *   author   : liuzhiqiangruc@126.com
 *   date     : 2016-03-28
 *   info     : 
 * ======================================================== */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "gblr.h"

void help() {
    fprintf(stderr, "\ngblr usage:        \n");
    fprintf(stderr, "\n./gblr -n <int> -m <int> -b <int> -r <double> -d <string> -t <string> -o <string>\n");
    fprintf(stderr, "     -n  tree capicity                   \n");
    fprintf(stderr, "     -m  max leaf node in per tree       \n");
    fprintf(stderr, "     -b  1:binary or else                \n");
    fprintf(stderr, "     -r  learning rate                   \n");
    fprintf(stderr, "     -d  train input file                \n");
    fprintf(stderr, "     -t  test  input file                \n");
    fprintf(stderr, "     -o  otuput dir                      \n");
}

int parse_command_line(GBMP *p, int argc, char *argv[]){
    double r = 0.0;
    int b = 0, n = 20, m = 10;
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
        else if (0 == strcmp(arg,"-r")){
            r = atof(argv[++i]);
        }
        else if (0 == strcmp(arg,"-d")){
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
    p->rate           = r;
    p->binary         = b;
    p->max_trees      = n;
    p->max_leaf_nodes = m;
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
    gbdt_train(gblr);
    gbdt_free(gblr);
    gblr = NULL;
    return 0;
}


