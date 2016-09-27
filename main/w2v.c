/* ========================================================
 *   Copyright (C) 2016 All rights reserved.
 *   
 *   filename : w2v.c
 *   author   : liuzhiqiangruc@126.com
 *   date     : 2016-09-26
 *   info     : 
 * ======================================================== */
#include <stdio.h>
#include <stdlib.h>
#include "w2v.h"

int main(int argc, char *argv[]){
    WV * wv = wv_create(argc, argv);
    if (! wv){
        return -1;
    }
    if(0 != wv_init(wv)){
        return -1;
    }
    wv_est(wv);
    wv_save(wv);
    wv_free(wv);
    wv = NULL;
    return 0;
}
