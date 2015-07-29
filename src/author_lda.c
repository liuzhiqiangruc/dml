/* ========================================================
 *   Copyright (C) 2015 All rights reserved.
 *   
 *   filename : author_lda.c
 *   author   : ***
 *   date     : 2015-04-03
 *   info     : 
 * ======================================================== */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "author_lda.h"


void help() {
    fprintf(stderr, "AuthorLda Command Usage: \n\n");
    fprintf(stderr, "./alda -a <float> -b <float> -k <int> -n <int> -s <int> -ad <string> -d <string> -phi <string> -o <string>: \n\n");
    fprintf(stderr, "       -a   alpha     doc-K   paramenter  \n");
    fprintf(stderr, "       -b   beta      K-V     paramenter  \n");
    fprintf(stderr, "       -k   topicx    tt-size             \n");
    fprintf(stderr, "       -n   niters    iteration num       \n");
    fprintf(stderr, "       -s   savestep  savestep            \n");
    fprintf(stderr, "       -ad  input     doc authors file    \n");
    fprintf(stderr, "       -d   input     input data file     \n");
    fprintf(stderr, "       -phi input     phi matrix input    \n");
    fprintf(stderr, "       -o   out_dir   output dir          \n");
}

int parse_command_line(ParamALda *p, int argc, char *argv[]) {
    int k = 50, n = 20, s = 10;
    double a = 0.05, b = 0.05;
    char *input = NULL;
    char *author = NULL;
    char *out_dir = "./";
    char *phi = NULL;

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
        else if (0 == strcmp(arg,"-s")){
            s = atoi(argv[++i]);
        }
        else if (0 == strcmp(arg,"-d")){
            input = argv[++i];
        }
        else if (0 == strcmp(arg,"-phi")){
            phi = argv[++i];
        }
        else if (0 == strcmp(arg,"-ad")){
            author = argv[++i];
        }
        else if (0 == strcmp(arg,"-o")){
            out_dir = argv[++i];
        }
        i += 1;
    }
    if (NULL == input || NULL == author) {
        fprintf(stderr,"please give author file and token file both. exit(-1)\n");
        return -1;
    }

    p->a           = a;
    p->b           = b;
    p->niters      = n;
    p->savestep    = s;
    p->k           = k;
    p->in_file     = input;
    p->phi_file    = phi;
    p->out_dir     = out_dir;
    p->author_file = author;

    fprintf(stderr, "alpha : %f\tbeta : %f\tniters : %d\ttopics : %d\tinput : %s\toutput : %s\n", \
            a, b, n, k, input, out_dir);
    return 0;
}

int main(int argc, char * argv[]) {
    AuthorLda * alda = create_author_lda();
    if (-1 == parse_command_line(&(alda->p), argc, argv)){
        help();
        return -1;
    }
    fprintf(stderr,"command line parse done\n");
    srand(time(NULL));

    if (-1 == init_author_lda(alda)) {
        return -1;
    }
    fprintf(stderr,"load data done\n");

    fprintf(stderr, "max author cnt is : %d\n", alda->MA);

    est_author_lda(alda);

    if (alda->p.niters % alda->p.savestep != 0){
        save_author_lda(alda, alda->p.niters);
    }

    free_author_lda(alda);
    alda = NULL;

    return 0;
}
