/* ========================================================
 *   Copyright (C) 2018 All rights reserved.
 *   
 *   filename : louvain.c
 *   author   : liuzhiqiangruc@126.com
 *   date     : 2018-01-04
 *   info     : 
 * ======================================================== */
#include <stdio.h>
#include <stdlib.h>
#include "louvain.h"
#include "hash.h"
#include "str.h"

#define PLEN 128

typedef struct _node {
    unsigned int count;     // node number of current cluster
    unsigned int clsid;     // the upper cluster id
    unsigned int sibling;   // the next node which belong to the same upper cluster 
    unsigned int eindex;    // first neighbor index 
    double kin;       // current node in weight
    double kout;      // current node out weight
    double clstot;    // nodes which belong to the same cluster have the same clstot;
} Node;

typedef struct _edge {
    unsigned int left;    // left <------ right
    unsigned int right;   
    unsigned int next;    // next neighbor index for node left
    double weight;        // edge weight from right to left
} Edge;

struct _louvain {
    unsigned int clen;
    unsigned int elen;
    unsigned int * cindex;
    Node * nodes;
    Edge * edges;
};

static void malloc_louvain(Louvain * lv){
    int i;
    lv->cindex = (unsigned int *)calloc(lv->clen, sizeof(unsigned int));
    memset(lv->cindex, -1, lv->clen * sizeof(unsigned int));
    lv->nodes  = (Node*)calloc(lv->clen, sizeof(Node));
    lv->edges  = (Edge*)calloc(lv->elen, sizeof(Edge));
    for (i = 0; i < lv->clen; i++){
        lv->nodes[i].eindex = -1;
    }
}

#define INIT_NODE(I) do{              \
    if (lv->cindex[I] == -1){         \
        lv->cindex[I] = I;            \
        lv->nodes[I].count = 1;       \
        lv->nodes[I].kin = 0;         \
        lv->nodes[I].sibling = I;     \
        lv->nodes[I].clsid = I;       \
    }                                 \
    lv->nodes[I].kout   += 1;         \
    lv->nodes[I].clstot += 1;         \
}while(0);

#define LINKEDGE(l,r,ei) do{                        \
    lv->edges[ei].left = l;                         \
    lv->edges[ei].right = r;                        \
    lv->edges[ei].weight = 1.0;                     \
    lv->edges[ei].next = lv->nodes[l].eindex;       \
    lv->nodes[l].eindex = ei;                       \
    ei += 1;                                        \
}while(0);

Louvain * create_louvain(const char * input){
    FILE * fp = NULL;
    if (NULL == (fp = fopen(input, "r"))){
        fprintf(stderr, "can not open input file \"%s\"\n", input);
        return NULL;
    }
    Hash * hs = hash_create(1 << 23, STRING);
    Louvain * lv = (Louvain*)calloc(1, sizeof(Louvain));
    char buffer[PLEN] = {0};
    char *string = buffer, *token;
    int l = 0, ei = 0, r = 0;
    while (NULL != fgets(buffer, PLEN, fp)){
        string = trim(buffer, 3);
        hash_add(strsep(&string, "\t"));
        hash_add(strsep(&string, "\t"));
        l += 1;
    }
    lv->clen = hash_cnt(hs);
    lv->elen = l * 2;
    malloc_louvain(lv);
    rewind(fp);
    while (NULL != fgets(buffer, PLEN, fp)){
        string = trim(buffer, 3);
        l = hash_find(hs, strsep(&string, "\t"));
        INIT_NODE(l)
        r = hash_find(hs, strsep(&string, "\t"));
        INIT_NODE(r)
        LINKEDGE(l,r,ei)
        LINKEDGE(r,l,ei)
    }
    fclose(fp);
    return lv;
}

static void first_stage(Louvain * lv){
}

static void second_stage(Louvain * lv){
}

int learn_louvain(Louvain * lv){
}

void save_louvain(Louvain * lv){
}

void free_louvain(Louvain * lv){
}

