/* ========================================================
 *   Copyright (C) 2016 All rights reserved.
 *   
 *   filename : d2v.c
 *   author   : liuzhiqiangruc@126.com
 *   date     : 2016-10-24
 *   info     : 
 * ======================================================== */

#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include "doc2vec.h"

int main(int argc, char *argv[]){
    D2V * d2v = d2v_create(argc, argv);
    srand(time(NULL));
    if (! d2v){
        return -1;
    }
    if(0 != d2v_init(d2v)){
        return -1;
    }
    fprintf(stderr, "init done, doc_size: %d, voc_size: %d, tk_size: %d\n", d2v_dsize(d2v), d2v_vsize(d2v), d2v_tsize(d2v));
    d2v_learn(d2v);
    d2v_save(d2v);
    d2v_free(d2v);
    d2v = NULL;
    return 0;
}
