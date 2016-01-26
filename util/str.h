/* ========================================================
 *   Copyright (C) 2014 All rights reserved.
 *   
 *   filename : str.h
 *   author   : liuzhiqiang01@baidu.com
 *   date     : 2014-09-19
 *   info     : simple string functions
 * ======================================================== */

#ifndef _STR_H
#define _STR_H

char *strsep(char **stringp, const char *delim);
char * trim(char *c, int mode);

#endif //STR_H
