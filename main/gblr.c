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
    fprintf(stderr, "\n./gblr -n <int> -m <int> -d <int> -l <int> -b <int> -a <double> -g <double> -r <double> -f <string> -s <string> -t <string> -y <string> -o <string>\n");
    fprintf(stderr, "     -n  tree capicity                   \n");
    fprintf(stderr, "     -m  max leaf node in per tree       \n");
    fprintf(stderr, "     -d  max depth of trees              \n");
    fprintf(stderr, "     -l  min instance num for each node  \n");
    fprintf(stderr, "     -b  1:binary or else                \n");
    fprintf(stderr, "     -a  node regulization               \n");
    fprintf(stderr, "     -g  weight regulization             \n");
    fprintf(stderr, "     -r  learning rate                   \n");
    fprintf(stderr, "     -f  train input file                \n");
    fprintf(stderr, "     -s  train init  file                \n");
    fprintf(stderr, "     -t  test  input file                \n");
    fprintf(stderr, "     -y  test  init  file                \n");
    fprintf(stderr, "     -o  otuput dir                      \n");
}

int parse_command_line(GBMP *p, int argc, char *argv[]){
    double r = 0.0;
    double w_reg = 0.0;
    double n_reg = 0.0;
    int b = 0, n = 10, m = 2, d = 1, l = 0;
    char * f = NULL;
    char * t = NULL;
    char * s = NULL;
    char * y = NULL;
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
        else if (0 == strcmp(arg,"-l")){
            l = atoi(argv[++i]);
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
        else if (0 == strcmp(arg,"-s")){
            s = argv[++i];
        }
        else if (0 == strcmp(arg,"-t")){
            t = argv[++i];
        }
        else if (0 == strcmp(arg,"-y")){
            y = argv[++i];
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
    p->min_node_ins   = l;
    p->nod_reg        = n_reg;
    p->wei_reg        = w_reg;
    p->rate           = r;
    p->train_input    = f;
    p->train_init     = s;
    p->test_input     = t;
    p->test_init      = y;
    p->out_dir        = o;
    return 0;
}

int main(int argc, char *argv[]) {
    GBMP p;
    if (-1 == parse_command_line(&p, argc, argv)){
        help();
        return -1;
    }
    fprintf(stderr, "command line paramenters parse done\n");
    fprintf(stderr, "    tree capacity       : %d\n", p.max_trees      );
    fprintf(stderr, "    max leaf nodes      : %d\n", p.max_leaf_nodes );
    fprintf(stderr, "    max depth           : %d\n", p.max_depth      );
    fprintf(stderr, "    min leaf node ins   : %d\n", p.min_node_ins   );
    fprintf(stderr, "    binary or not       : %d\n", p.binary         );
    fprintf(stderr, "    node regulization   : %f\n", p.nod_reg        );
    fprintf(stderr, "    weit regulization   : %f\n", p.wei_reg        );
    fprintf(stderr, "    learning rate       : %f\n", p.rate           );
    fprintf(stderr, "    trainning data      : %s\n", p.train_input    );
    fprintf(stderr, "    testing   data      : %s\n", p.test_input     );
    fprintf(stderr, "    trainning init      : %s\n", p.train_init     );
    fprintf(stderr, "    testing   init      : %s\n", p.test_init      );
    fprintf(stderr, "    output dir          : %s\n", p.out_dir        );

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
