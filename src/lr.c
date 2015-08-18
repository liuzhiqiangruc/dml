/* ========================================================
 *   Copyright (C) 2015 All rights reserved.
 *   
 *   filename : lr.c
 *   author   : ***
 *   date     : 2015-08-18
 *   info     : 
 * ======================================================== */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lr.h"


void help() {
    fprintf(stderr, "\nLR [Logistic Regression] usage:        \n");
    fprintf(stderr, "\n./lr -f <string> -a <double> -r <int>  \n");
    fprintf(stderr, "     -f  data input file                 \n");
    fprintf(stderr, "     -t  test input file                 \n");
    fprintf(stderr, "     -o  otuput dir                      \n");
    fprintf(stderr, "     -a  regularized paramenter          \n");
    fprintf(stderr, "     -b  1:binary or else                \n");
    fprintf(stderr, "     -r  1:L1 Norm; 2: L2 Norm           \n\n");
}

int parse_command_line(LRParam *p, int argc, char *argv[]){
    double a = 0;
    int b = 0, r = 1, n = 10, s = 10;
    char * f = NULL;
    char * t = NULL;
    char * o = NULL;
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
    p->binary   = b;
    p->method   = r;
    p->niters   = n;
    p->savestep = s;
    p->in_file  = f;
    p->te_file  = t;
    p->out_dir  = o;

    fprintf(stderr, "lambda : %f\tniters : %d\tsavestep: %d\tinput : %s\toutput : %s\n", \
            a, n, s, f, o);

    return 0;

}


int main(int argc, char *argv[]) {
    LR * lr = create_lr_model();
    if (-1 == parse_command_line(&(lr->p), argc, argv)){
        help();
        goto except;
    }
    fprintf(stderr, "command line parse done\n");
    if (-1 == init_lr(lr)){
        goto except;
    }
    fprintf(stderr, "load data done\n");
    fprintf(stderr, "train: %d  test: %d lenx: %d\n", lr->train_ds->r, lr->test_ds->r, lr->c);
    learn_lr(lr);
    save_lr(lr, lr->p.niters);
    free_lr(lr);
    lr = NULL;
    return 0;

except:
    free_lr(lr);
    lr = NULL;
    return -1;
}


