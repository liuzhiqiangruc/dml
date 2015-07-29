/* ========================================================
 *   Copyright (C) 2015 All rights reserved.
 *   
 *   filename : idmap.c
 *   author   : liuzhiqiang01@baidu.com
 *   date     : 2015-03-23
 *   info     : implementation of idmap
 * ======================================================== */

#include <stdlib.h>
#include <string.h>
#include "idmap.h"

typedef struct {
    char *key;
    int value;
} Pair;


void pfree(Pair *p) {
    free(p->key);
    p->key = NULL;
    free(p);
}

int pcmp(Pair *p1, Pair *p2) {
    return strcmp(p1->key, p2->key);
}

IdMap *idmap_create() {
    return set_create((CMP_FN) pcmp, (FREE_FN) pfree);
}


int idmap_size(IdMap *idmap) {
    return idmap->size;
}

void idmap_add(IdMap *idmap, char *key, int value) {
    Pair *p = malloc(sizeof(Pair));
    p->key = key;
    p->value = value;
    set_add(idmap, p);
}

void idmap_update_value(IdMap * idmap, char * key, int value){
    Pair * p = (Pair*)malloc(sizeof(Pair));
    p->key = key;
    RBNode * node = node_find(idmap->rb, p);
    if (node == NULL){
        return ;
    }
    free(p);
    ((Pair *)node->pData)->value = value;
}


int idmap_get_value(IdMap *idmap, char *key) {
    Pair * p = (Pair*)malloc(sizeof(Pair));
    p->key = key;
    RBNode *node = node_find(idmap->rb, p);
    if (node == NULL) {
        return -1;
    }
    free(p);
    return ((Pair *)node->pData)->value;
}


void idmap_free(IdMap *idmap) {
    set_free(idmap);
}

