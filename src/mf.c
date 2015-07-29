/* ========================================================
 *   Copyright (C) 2015 All rights reserved.
 *   
 *   filename : mf.c
 *   author   : ***
 *   date     : 2015-05-19
 *   info     : 
 * ======================================================== */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "mf.h"

void help() {
#ifdef PMF
    fprintf(stderr, "PMF Command Usage: \n");
    fprintf(stderr, "./pmf -a <float> -b <float> -k <int> -n <int> -u <int> -s <int> -d <string> -t <string> -o <string>: \n\n");
#elif BMF
    fprintf(stderr, "BMF Command Usage: \n");
    fprintf(stderr, "./bmf -a <float> -b <float> -k <int> -n <int> -u <int> -s <int> -d <string> -t <string> -o <string>: \n\n");
#endif
    fprintf(stderr, "       -a  alpha     learn rate          \n");
    fprintf(stderr, "       -b  beta      penty   paramenter  \n");
    fprintf(stderr, "       -k  latent    feature count       \n");
    fprintf(stderr, "       -n  niters    iteration num       \n");
    fprintf(stderr, "       -u  nbias     biase burn num      \n");
    fprintf(stderr, "       -s  savestep  savestep            \n");
    fprintf(stderr, "       -d  input     input data file     \n");
    fprintf(stderr, "       -t  input     input test file     \n");
    fprintf(stderr, "       -o  out_dir   output dir          \n");
}


int parse_command_line(ParaMF * p, int argc, char *argv[]) {
    int k = 50, n = 20, s = 0, u = 30;
    double a = 0.05, b = 0.01;
    char *input = NULL;
    char *test = NULL;
    char *out_dir = "./";

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
            b = atof(argv[++i]);
        }
        else if (0 == strcmp(arg,"-k")){
            k = atoi(argv[++i]);
        }
        else if (0 == strcmp(arg,"-n")){
            n = atoi(argv[++i]);
        }
        else if (0 == strcmp(arg,"-u")){
            u = atoi(argv[++i]);
        }
        else if (0 == strcmp(arg,"-s")){
            s = atoi(argv[++i]);
        }
        else if (0 == strcmp(arg,"-d")){
            input = argv[++i];
        }
        else if (0 == strcmp(arg,"-t")){
            test = argv[++i];
        }
        else if (0 == strcmp(arg,"-o")){
            out_dir = argv[++i];
        }
        i += 1;
    }
    if (NULL == input) {
        fprintf(stderr,"no input data exit(-1)\n");
        return -1;
    }
    if (s == 0) {
        s = n >> 1;
    }
    p->a        = a;
    p->b        = b;
    p->niters   = n;
    p->nbias    = u;
    p->savestep = s;
    p->k        = k;
    p->in_file  = input;
    p->te_file  = test;
    p->out_dir  = out_dir;

    return 0;
}


int main(int argc, char *argv[]){
    MF * mf = create_mf();
    if (-1 == parse_command_line(&(mf->p), argc, argv)){
        help();
        return -1;
    }
    fprintf(stderr, "command line parse done\n");
    srand(time(NULL));
    if (-1 == init_mf(mf)){
        return -1;
    }
    fprintf(stderr, "load data done\n");
    fprintf(stderr, "user: %d  item: %d  mean : %f   min : %f  max : %f\n", \
                    mf->U, mf->I, mf->mu, mf->min_s, mf->max_s);
    save_mf(mf, 0);
    est_mf(mf);
    save_mf(mf, mf->p.niters);
    free_mf(mf);
    mf = NULL;
    return 0;
}
