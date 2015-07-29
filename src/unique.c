/* ========================================================
 *   Copyright (C) 2015 All rights reserved.
 *
 *   filename : unique.c
 *   author   : ***
 *   date     : 2015-03-20
 *   info     :
 * ======================================================== */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "set.h"

#define LINE_LEN  1024

void s_free(void * s){
    free(s);
}

void help() {
    fprintf(stderr, "unique usage:\n");
    fprintf(stderr, "   ./unique unique_file[string]\n");
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        help();
        return -1;
    }

    Set *set = set_create((CMP_FN)strcmp, (FREE_FN)s_free);
    char * filename = argv[1];

    FILE * f = NULL;
    if (NULL == (f = fopen(filename,"r"))){
        fprintf(stderr,"file does not exist\n");
        return -1;
    }
    char buffer[LINE_LEN];

    while(fgets(buffer, LINE_LEN, f) != NULL){
        int len = strlen(buffer);
        char *s = (char *)malloc(sizeof(char) * (len+1));
        memcpy(s, buffer, len);
        if (s[len - 1] == '\n'){
            s[len - 1] = 0;
        } else {
            s[len] = 0;
        }
        set_add(set, s);
    }
    reset(set);
    while(!isEnd(set)) {
        printf("%s\n", (char*)set_value(set));
        set_next(set);
    }

    set_free(set);

    return 0;
}

