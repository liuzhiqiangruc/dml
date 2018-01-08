/* ========================================================
 *   Copyright (C) 2018 All rights reserved.
 *   
 *   filename : louvain.c
 *   author   : ***
 *   date     : 2018-01-08
 *   info     : 
 * ======================================================== */

#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "louvain.h"

void help(){
    fprintf(stderr, "lv usage:\n");
    fprintf(stderr, "./lv -input\n");
}

int main(int argc, char *argv[]){
    Louvain * lv = create_louvain(argv[1]);
    learn_louvain(lv);
    return 0;
}
