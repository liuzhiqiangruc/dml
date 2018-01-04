/* ========================================================
 *   Copyright (C) 2018 All rights reserved.
 *   
 *   filename : louvain.h
 *   author   : ***
 *   date     : 2018-01-03
 *   info     : 
 * ======================================================== */

#ifndef _LOUVAIN_H
#define _LOUVAIN_H

typedef struct _louvain Louvain;

Louvain * create_louvain(const char * input);
int learn_louvain(Louvain * lv);
void save_louvain(Louvain * lv);
void free_louvain(Louvain * lv);

#endif //LOUVAIN_H
