/* ========================================================
 *   Copyright (C) 2014 All rights reserved.
 *   
 *   filename : str.c
 *   author   : liuzhiqiang01@baidu.com
 *   date     : 2014-09-19
 *   info     : simple string functions
 * ======================================================== */

#include <stdlib.h>
#include <string.h>
#include "str.h"

static inline int charmask(unsigned char *input, int len, char *mask)
{
    unsigned char *end;
    unsigned char c;
    int result = 0;

    memset(mask, 0, 256);
    for (end = input + len; input < end; input++) {
        c=*input;
        if ((input + 3 < end) && input[1] == '.' && input[2] == '.'
                && input[3] >= c) {
            memset(mask + c, 1, input[3] - c + 1);
            input += 3;
        } else if ((input + 1 < end) && input[0] == '.' && input[1] == '.') {
            if (end-len >= input) { 
                result = -1;
                continue;
            }
            if (input + 2 >= end) { 
                result = -1;
                continue;
            }
            if (input[-1] > input[2]) { 
                result = -1;
                continue;
            }
            result = -1;
            continue;
        } else {
            mask[c] = 1;
        }
    }
    return result;
}

char * trim(char *c, int mode){
    if (!c)
        return NULL;
    register int i;
    int len = strlen(c) + 1;
    int trimmed = 0;
    char mask[256];
    charmask((unsigned char*)" \n\r\t\v\0", 6, mask);
    if (mode & 1) {
        for (i = 0; i < len; i++) {
            if (mask[(unsigned char)c[i]]) {
                trimmed++;
            } else {
                break;
            }
        }
        len -= trimmed;
        c += trimmed;
    }
    if (mode & 2) {
        for (i = len - 1; i >= 0; i--) {
            if (mask[(unsigned char)c[i]]) {
                len--;
            } else {
                break;
            }
        }
    }
    c[len] = '\0';
    return c;
}

char *strsep(char **stringp, const char *delim) {
    char *s;
    const char *spanp;
    int c, sc;
    char *tok;
    if ((s = *stringp)== NULL)
        return (NULL);
    for (tok = s;;) {
        c = *s++;
        spanp = delim;
        do {
            if ((sc =*spanp++) == c) {
                if (c == 0)
                    s = NULL;
                else
                    s[-1] = 0;
                *stringp = s;
                return (tok);
            }
        } while (sc != 0);
    }
}
