/* ========================================================
 *   Copyright (C) 2016 All rights reserved.
 *   
 *   filename : tm.c
 *   author   : liuzhiqiangruc@126.com
 *   date     : 2016-07-19
 *   info     : 
 * ======================================================== */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/types.h>
#include <unistd.h>
#include "tm.h"

int main(int argc, char *argv[]){
    TM * tm = tm_create(argc, argv);
    if (!tm){
        return -1;
    }
    srand(time(NULL) + (unsigned int) getpid());
    if (0 != tm_init(tm)){
        return -1;
    }
    tm_est(tm);
    tm_save(tm, -1);
    tm_free(tm);
    tm = NULL;
    return 0;
}
