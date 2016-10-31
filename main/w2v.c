/* ========================================================
 *   Copyright (C) 2016 All rights reserved.
 *   
 *   filename : w2v.c
 *   author   : liuzhiqiangruc@126.com
 *   date     : 2016-09-26
 *   info     : 
 * ======================================================== */
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include "word2vec.h"

int main(int argc, char *argv[]){
    W2V * w2v = w2v_create(argc, argv);
    //srand(time(NULL));
    if (! w2v){
        return -1;
    }
    if(0 != w2v_init(w2v)){
        return -1;
    }
    fprintf(stderr, "init done, doc_size: %d, voc_size: %d, tk_size: %d\n", w2v_dsize(w2v), w2v_vsize(w2v), w2v_tsize(w2v));
    w2v_learn(w2v);
    w2v_save(w2v);
    w2v_free(w2v);
    w2v = NULL;
    return 0;
}
