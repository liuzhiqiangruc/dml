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

static void hold_nei_comm(int ** nei_comm_id, double ** nei_comm_weight, Hash * in, int len){
    if (*nei_comm_id == NULL){
        *nei_comm_id = (int*)calloc(hsize, sizeof(int));
        *nei_comm_weight = (double*)calloc(hsize, sizeof(double));
    }
    else{
        int * tmp = (int*)calloc(hash_size(in), sizeof(int));
        memmove(tmp, *nei_comm_id, len * sizeof(int));
        free(*nei_comm_id);
        *nei_comm_id = tmp;
        double * tmp1 = (double*)calloc(hash_size(in), sizeof(double));
        memmove(tmp1, *nei_comm_weight, len * sizeof(double));
        free(*nei_comm_weight);
        *nei_comm_weight = tmp1;
    }
}

static int first_stage(Louvain * lv){
    int i, j, ci, cci, ei, wi, wci, hsize, cp = 0;
    int nei_comm_cnt = 0;
    int id, maxId;
    int keepgoing = 0;
    // this needs init before using
    int * nei_comm_id = NULL;
    double kv, wei, tot;
    double deltaQ, maxDeltaQ;
    double * nei_comm_weight = NULL;
    Hash * in = hash_create(1 << 16, INT);
    hsize = hash_size(in);
    hold_nei_comm(&nei_comm_id, &nei_comm_weight, NULL, hsize);
    while (1){
        for (i = 0; i < lv->clen; i++){
            ci  = lv->cindex[i];
            kv  = lv->nodes[ci].kin + lv->nodes[ci].kout;
            cci = lv->nodes[ci].clsid;
            ei  = lv->nodes[ci].eindex;
            hash_clean(in);
            memest(nei_comm_id, 0, sizeof(int) * hsize);
            memset(nei_comm_weight, 0, sizeof(double) * hsize);
            while(-1 != ei){
                wi  = lv->edges[ei].right;
                wei = lv->edges[ei].weight;
                wci = lv->nodes[wi].clsid;
                id = hash_add(in, wci);
                if (id >= hsize){
                    hold_nei_comm(&nei_comm_id, &nei_comm_weight, in, hsize);
                    hsize = hash_size(in);
                }
                nei_comm_id[id] = wci;
                nei_comm_weight[id] += wei;
                ei  = lv->edges[ei].next;
            }
            maxDeltaQ = 0.0;
            for (j = 0; j < id; j++){
                deltaQ = nei_comm_weight[j] - kv * lv->nodes[nei_comm_id[j]].clstot / lv->elen;
                if (deltaQ > maxDeltaQ){
                    maxDeltaQ = deltaQ;
                    maxId = nei_comm_id[j];
                }
            }
            // change the comm for current node
            if (maxDeltaQ > 0.0 and maxId != cci){
                // add current node to commu maxId
                lv->nodes[ci].clsid = maxId;
                lv->nodes[ci].sibling = lv->nodes[maxId].sibling;
                lv->nodes[maxId].sibling = ci;
                lv->nodes[ci].clsid = maxId;
                lv->nodes[maxId].count += lv->nodes[ci].count;
                keepgoing = 1;
            }
            else{
                break;
            }
        }
    }
    return keepgoing;
}

static void second_stage(Louvain * lv){
}

int learn_louvain(Louvain * lv){
    while (first_stage(lv)){
        second_stage(lv);
    }
    return 0;
}

void save_louvain(Louvain * lv){
}

void free_louvain(Louvain * lv){
}

