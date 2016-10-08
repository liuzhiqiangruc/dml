/* ========================================================
 *   Copyright (C) 2016 All rights reserved.
 *   
 *   filename : repo.c
 *   author   : liuzhiqiangruc@126.com
 *   date     : 2016-10-08
 *   info     : 
 * ======================================================== */

#include <string.h>
#include "repo.h"

void progress(FILE * fp, double s, double b){
    static int ls = 0;
    double sb = b / s;
    int n = sb * 50;
    char buf[51] = {'\0'};
    if (n > ls){
        memset(buf, '=', n - 1);
        buf[n - 1] = '>';
        fprintf(fp, "[%-50s]%6.2f%%\r", buf, sb > 1 ? 100 : sb * 100);
        fflush(stdout);
        ls = n;
    }
    if (sb >= 1){
        fprintf(fp, "\n");
        ls = 0;
    }
}
