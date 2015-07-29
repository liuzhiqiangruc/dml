/* ========================================================
 *   Copyright (C) 2015 All rights reserved.
 *   
 *   filename : joint_lda.c
 *   author   : ***
 *   date     : 2015-03-23
 *   info     : 
 * ======================================================== */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "joint_lda.h"


void help(){
    fprintf(stderr,"\nJoint Lda Command Usage: \n\n");
    fprintf(stderr,"./jlda -a <float> -a1 <float> -a2 <float> -b1 <float> -b2 <float> -k <int> -k1 <int> -k2 <int> -n <int> -s <int> -d <string> -o <string>: \n\n");
    fprintf(stderr,"       -a  alpha     doc-K   paramenter  \n");
    fprintf(stderr,"       -a1 alpha     K-K1    paramenter  \n");
    fprintf(stderr,"       -a2 alpha     K-K2    paramenter  \n");
    fprintf(stderr,"       -b1 beta1     K1-v1   paramenter  \n");
    fprintf(stderr,"       -b2 beta2     K2-v2   paramenter  \n");
    fprintf(stderr,"       -k  topicx    tt-size             \n");
    fprintf(stderr,"       -k1 topics    topic1 size         \n");
    fprintf(stderr,"       -k2 topics    topic2 size         \n");
    fprintf(stderr,"       -n niters     iteration num       \n");
    fprintf(stderr,"       -s savestep   savestep            \n");
    fprintf(stderr,"       -d input      input data file     \n");
    fprintf(stderr,"       -o out_dir    output dir          \n");
}

int parse_command_line(ParamJointLda * p, int argc, char * argv[]){
    int k = 50, k1 = 50, k2 = 50, n = 20, s = 10;
    double a = 0.05, a1 = 0.05, a2 = 0.05, b1 = 0.05, b2 = 0.05;
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
        else if (0 == strcmp(arg,"-a1")){
            a1 = atof(argv[++i]);
        }
        else if (0 == strcmp(arg,"-a2")){
            a2 = atof(argv[++i]);
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
        else if (0 == strcmp(arg,"-k1")){
            k1 = atoi(argv[++i]);
        }
        else if (0 == strcmp(arg,"-k2")){
            k2 = atoi(argv[++i]);
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
    p->a1 = a1;
    p->a2 = a2;
    p->b1 = b1;
    p->b2 = b2;
    p->niters = n;
    p->savestep = s;
    p->k = k;
    p->k1 = k1;
    p->k2 = k2;
    p->in_file = input;
    p->out_dir = out_dir;

    return 0;
}

int main(int argc, char * argv[]){
    JointLda * jlda = create_joint_lda();
    if (-1 == parse_command_line(&(jlda->p), argc, argv)){
        help();
        return -1;
    }
    fprintf(stderr,"command line parse done\n");

    srand(time(NULL));

    if (-1 == init_joint_lda(jlda)){
        return -1;
    }
    fprintf(stderr,"load data done\n");

    fprintf(stderr,"doc: %d    v1: %d     v2: %d\n", jlda->d, jlda->v1, jlda->v2);
    est_joint_lda(jlda);
    free_joint_lda(jlda);
    jlda = NULL;
    return 0;
}
