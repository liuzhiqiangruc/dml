/* ========================================================
 *   Copyright (C) 2015 All rights reserved.
 *   
 *   filename : gr.c
 *   author   : liuzhiqiang01@baidu.com
 *   date     : 2015-08-27
 *   info     : guassian regression implementation
 *              using regression framework
 * ======================================================== */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "regression.h"

typedef Regression GR;


void help() {
    fprintf(stderr, "\nGR [Guassian Regression] usage:        \n");
    fprintf(stderr, "\n./gr -a <double> -l <double> -b <int> -r <int> -n <int> -s <int> -f <string> -t <string> -o <string\n");
    fprintf(stderr, "     -a  regularized paramenter          \n");
    fprintf(stderr, "     -l  Convergence tolerance           \n");
    fprintf(stderr, "     -b  1:binary or else                \n");
    fprintf(stderr, "     -r  1:L1 Norm; 2: L2 Norm           \n");
    fprintf(stderr, "     -n  max iteration count             \n");
    fprintf(stderr, "     -s  savestep                        \n");
    fprintf(stderr, "     -f  train input file                \n");
    fprintf(stderr, "     -t  test  input file                \n");
    fprintf(stderr, "     -o  otuput dir                      \n");
}

int parse_command_line(RP *p, int argc, char *argv[]){
    double a = 0, l = 1e-5;
    int b = 0, r = 1, n = 10, s = 10;
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
        if (0 == strcmp(arg,"-a")){
            a = atof(argv[++i]);
        }
        else if (0 == strcmp(arg,"-l")){
            l = atof(argv[++i]);
        }
        else if (0 == strcmp(arg,"-b")){
            b = atoi(argv[++i]);
        }
        else if (0 == strcmp(arg,"-n")){
            n = atoi(argv[++i]);
        }
        else if (0 == strcmp(arg,"-s")){
            s = atoi(argv[++i]);
        }
        else if (0 == strcmp(arg,"-r")){
            r = atoi(argv[++i]);
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
    if (r != 1 && r != 2){
        fprintf(stderr, "method must be 1, or 2\n");
        return -1;
    }
    if (b != 0 && b != 1){
        fprintf(stderr, "binary must be 0, or 1\n");
        return -1;
    }
    p->lambda   = a;
    p->ftoler   = l;
    p->binary   = b;
    p->method   = r;
    p->niters   = n;
    p->savestep = s;
    p->in_file  = f;
    p->te_file  = t;
    p->out_dir  = o;

    return 0;

}


int main(int argc, char *argv[]) {
    GR *gr = create_regression();
    if (-1 == parse_command_line(&(gr->p), argc, argv)){
        help();
        goto except;
    }
    fprintf(stderr, "command line parse done\n");
    if (-1 == init_regression(gr)){
        goto except;
    }
    fprintf(stderr, "load data done\n");
    fprintf(stderr, "train: %d, lenx: %d\n", gr->train_ds->r, gr->c);
    if (gr->test_ds){
        fprintf(stderr, " test: %d\n", gr->test_ds->r);
    }
    learn_regression(gr);
    save_regression(gr, gr->p.niters);
    free_regression(gr);
    gr = NULL;
    return 0;

except:
    free_regression(gr);
    gr = NULL;
    return -1;
}

