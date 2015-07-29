/* ========================================================
 *   Copyright (C) 2015 All rights reserved.
 *   
 *   filename : idmap.h
 *   author   : liuzhiqiang01@baidu.com
 *   date     : 2015-03-23
 *   info     : give each unique string an int Id
 * ======================================================== */

#ifndef _MAP_H
#define _MAP_H

#include "set.h"

/* ---------------------------------
 * define IdMap just Using Set 
 * --------------------------------- */
typedef Set IdMap;

/* ---------------------------------
 * IdMap operation
 * --------------------------------- */
IdMap *idmap_create();
int  idmap_size(IdMap *idmap);
void idmap_add(IdMap *idmap, char *key, int value);
void idmap_update_value(IdMap * idmap, char * key, int value);
int  idmap_get_value(IdMap *idmap, char *key);
void idmap_free(IdMap *idmap);

#endif //MAP_H

