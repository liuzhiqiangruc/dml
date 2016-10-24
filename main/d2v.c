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
#include "d2v.h"

int main(int argc, char *argv[]){
    DV * dv = dv_create(argc, argv);
 //   srand(time(NULL));
    if (! dv){
        return -1;
    }
    if(0 != dv_init(dv)){
        return -1;
    }
    fprintf(stderr, "init done, doc_size: %d, voc_size: %d, tk_size: %d\n", dv_dsize(dv), dv_vsize(dv), dv_tsize(dv));
    dv_est(dv);
    dv_save(dv);
    dv_free(dv);
    dv = NULL;
    return 0;
}
