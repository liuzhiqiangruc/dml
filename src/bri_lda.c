/* ========================================================
 *   Copyright (C) 2015 All rights reserved.
 *   
 *   filename : bri_lda.c
 *   author   : ***
 *   date     : 2015-04-02
 *   info     : 
 * ======================================================== */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "bri_lda.h"


void help(){
    fprintf(stderr,"\nBridge Lda Command Usage: \n\n");
    fprintf(stderr,"./blda -a <float> -b1 <float> -b2 <float> -k <int> -n <int> -s <int> -d <string> -o <string>\n");
    fprintf(stderr,"       -a  alpha     doc-K  paramenter   \n");
    fprintf(stderr,"       -b1 beta1     K-v1   paramenter   \n");
    fprintf(stderr,"       -b2 beta2     K-v2   paramenter   \n");
    fprintf(stderr,"       -k  topicx    tt-size             \n");
    fprintf(stderr,"       -n niters     iteration num       \n");
    fprintf(stderr,"       -s savestep   savestep            \n");
    fprintf(stderr,"       -d input      input data file     \n");
    fprintf(stderr,"       -o out_dir    output dir          \n");
}

int parse_command_line(ParamBriLda * p, int argc, char * argv[]){
    int k = 50, n = 20, s = 10;
    double a = 0.05, b1 = 0.05, b2 = 0.05;
    char *input = NULL;
    char *out_dir = "./";

    int i = 0;
    char * arg = NULL;

    if ((argc & 1) == 0){
        fprintf(stderr, "command line not well formatted\n");
        return -1;
    }

    while (i < argc){
        arg = argv[i];
        if (0 == strcmp(arg,"-a")){
            a = atof(argv[++i]);
        }
        else if (0 == strcmp(arg,"-b1")){
            b1 = atof(argv[++i]);
        }
        else if (0 == strcmp(arg,"-b2")){
            b2 = atof(argv[++i]);
        }
        else if (0 == strcmp(arg,"-k")){
            k = atoi(argv[++i]);
        }
        else if (0 == strcmp(arg,"-n")){
            n = atoi(argv[++i]);
        }
        else if (0 == strcmp(arg,"-s")){
            s = atoi(argv[++i]);
        }
        else if (0 == strcmp(arg,"-d")){
            input = argv[++i];
        }
        else if (0 == strcmp(arg,"-o")){
            out_dir = argv[++i];
        }
        i += 1;
    }
    if (NULL == input){
        fprintf(stderr,"no input data exit(-1)\n");
        return -1;
    }

    p->a = a;
    p->b1 = b1;
    p->b2 = b2;
    p->niters = n;
    p->savestep = s;
    p->k = k;
    p->in_file = input;
    p->out_dir = out_dir;
    fprintf(stderr, "alpha : %f\tbeta1 : %f\tbeta2 : %fniters : %d\ttopics : %d\tinput : %s\toutput : %s\n", \
            a, b1, b2, n, k, input, out_dir);

    return 0;
}

int main(int argc, char * argv[]){
    BriLda * blda = create_bri_lda();
    if (-1 == parse_command_line(&(blda->p), argc, argv)){
        help();
        return -1;
    }
    fprintf(stderr,"command line parse done\n");

    srand(time(NULL));

    if (-1 == init_bri_lda(blda)){
        return -1;
    }
    fprintf(stderr,"load data done\n");

    fprintf(stderr,"doc: %d    v1: %d     v2: %d\n", blda->d, blda->v1, blda->v2);
    est_bri_lda(blda);
    free_bri_lda(blda);
    blda = NULL;
    return 0;
}
