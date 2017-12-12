/* ========================================================
 *   Copyright (C) 2017 All rights reserved.
 *   
 *   filename : regcfg.c
 *   author   : ***
 *   date     : 2017-12-11
 *   info     : 
 * ======================================================== */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "regcfg.h"


int parse_command_line(REGP *p, int argc, char *argv[]){
    double a = 0, g = 0, l = 1e-5;
    int b = 0, r = 1, n = 10, s = 10, k = 0;
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
        else if (0 == strcmp(arg,"-g")){
            g = atof(argv[++i]);
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
        else if (0 == strcmp(arg,"-k")){
            k = atoi(argv[++i]);
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
    p->alpha       = a;
    p->gamma       = g;
    p->toler       = l;
    p->b           = b;
    p->r           = r;
    p->k           = k;
    p->n           = n;
    p->s           = s;
    p->train_input = f;
    p->test_input  = t;
    p->out_dir     = o;

    return 0;
}


