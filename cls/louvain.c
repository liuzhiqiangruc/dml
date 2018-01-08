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
#include <string.h>
#include "louvain.h"
#include "hash.h"
#include "str.h"

#define PLEN 128

typedef struct _node {
    int    count;        // node number of current cluster int    clsid;        // the upper cluster id
    int    clsid;        // the upper cluster id
    int    sibling;      // the next node which belong to the same upper cluster 
    int    eindex;       // first neighbor index 
    double kin;       // current node in weight
    double kout;      // current node out weight
    double clstot;    // nodes which belong to the same cluster have the same clstot;
} Node;

typedef struct _edge {
    int    left;    // left <------ right
    int    right;   
    int    next;    // next neighbor index for node left
    double weight;  // edge weight from right to left
} Edge;

struct _louvain {
    int  clen;
    int  elen;
    int  * cindex;
    Node * nodes;
    Edge * edges;
};

static void malloc_louvain(Louvain * lv){
    int i;
    lv->cindex = (int *)calloc(lv->clen, sizeof(int));
    memset(lv->cindex, -1, lv->clen * sizeof(int));
    lv->nodes  = (Node*)calloc(lv->clen, sizeof(Node));
    lv->edges  = (Edge*)calloc(lv->elen, sizeof(Edge));
    for (i = 0; i < lv->clen; i++){
        lv->nodes[i].eindex = -1;
    }
}

static void INIT_NODE(Louvain * lv, int I){
    if (lv->cindex[I] == -1){         
        lv->cindex[I] = I;            
        lv->nodes[I].count = 1;       
        lv->nodes[I].kin = 0;         
        lv->nodes[I].sibling = I;     
        lv->nodes[I].clsid = I;       
    }                                 
    lv->nodes[I].kout   += 1;         
    lv->nodes[I].clstot += 1;         

}

/*
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
*/

static void LINKEDGE(Louvain * lv, int l, int r, int ei){
    lv->edges[ei].left = l;                         
    lv->edges[ei].right = r;                        
    lv->edges[ei].weight = 1.0;                     
    lv->edges[ei].next = lv->nodes[l].eindex;       
    lv->nodes[l].eindex = ei;                       
    ei += 1;                                        
}

/*
#define LINKEDGE(l,r,ei) do{                        \
    lv->edges[ei].left = l;                         \
    lv->edges[ei].right = r;                        \
    lv->edges[ei].weight = 1.0;                     \
    lv->edges[ei].next = lv->nodes[l].eindex;       \
    lv->nodes[l].eindex = ei;                       \
    ei += 1;                                        \
}while(0);
*/

Louvain * create_louvain(const char * input){
    FILE * fp = NULL;
    if (NULL == (fp = fopen(input, "r"))){
        fprintf(stderr, "can not open input file \"%s\"\n", input);
        return NULL;
    }
    Hash * hs = hash_create(1 << 23, STRING);
    Louvain * lv = (Louvain*)calloc(1, sizeof(Louvain));
    char buffer[PLEN] = {0};
    char *string = buffer;
    int l = 0, ei = 0, r = 0;
    while (NULL != fgets(buffer, PLEN, fp)){
        string = trim(buffer, 3);
        hash_add(hs, strsep(&string, "\t"));
        hash_add(hs, strsep(&string, "\t"));
        l += 1;
    }
    lv->clen = hash_cnt(hs);
    lv->elen = l * 2;
    malloc_louvain(lv);
    rewind(fp);
    while (NULL != fgets(buffer, PLEN, fp)){
        string = trim(buffer, 3);
        l = hash_find(hs, strsep(&string, "\t"));
        INIT_NODE(lv, l);
        r = hash_find(hs, strsep(&string, "\t"));
        INIT_NODE(lv, r);
        LINKEDGE(lv, l,r,ei);
        ei += 1;
        LINKEDGE(lv, r,l,ei);
        ei += 1;
    }
    fclose(fp);
    return lv;
}

static void hold_nei_comm(int ** nei_comm_id, double ** nei_comm_weight, Hash * in, int hsize){
    if (*nei_comm_id == NULL){
        *nei_comm_id = (int*)calloc(hsize, sizeof(int));
        *nei_comm_weight = (double*)calloc(hsize, sizeof(double));
    }
    else{
        int * tmp = (int*)calloc(hash_size(in), sizeof(int));
        memmove(tmp, *nei_comm_id, hsize * sizeof(int));
        free(*nei_comm_id);
        *nei_comm_id = tmp;
        double * tmp1 = (double*)calloc(hash_size(in), sizeof(double));
        memmove(tmp1, *nei_comm_weight, hsize * sizeof(double));
        free(*nei_comm_weight);
        *nei_comm_weight = tmp1;
    }
}

static int first_stage(Louvain * lv){
    int i, j, ci, cid, ei, wi, wci, hsize;
    int id, maxId = -1;
    int keepgoing = 0;
    int need_stage_two = 0;
    int * nei_comm_id = NULL;
    double kv, wei;
    double deltaQ, maxDeltaQ;
    double * nei_comm_weight = NULL;
    Hash * in = hash_create(1 << 16, INT);
    hsize = hash_size(in);
    hold_nei_comm(&nei_comm_id, &nei_comm_weight, NULL, hsize);
    while (1){
        keepgoing = 0;
        for (i = 0; i < lv->clen; i++){
            ci  = lv->cindex[i];
            kv  = lv->nodes[ci].kin + lv->nodes[ci].kout;
            cid = lv->nodes[ci].clsid;
            ei  = lv->nodes[ci].eindex;
            hash_clean(in);
            memset(nei_comm_id, 0, sizeof(int) * hsize);
            memset(nei_comm_weight, 0, sizeof(double) * hsize);
            id = -1;
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
            for (j = 0; j <= id; j++){
                if (cid == nei_comm_id[j]){
                    deltaQ = nei_comm_weight[j] - kv * (lv->nodes[nei_comm_id[j]].clstot - kv) / lv->elen;
                }
                else{
                    deltaQ = nei_comm_weight[j] - kv * lv->nodes[nei_comm_id[j]].clstot / lv->elen;
                }
                if (deltaQ > maxDeltaQ){
                    maxDeltaQ = deltaQ;
                    maxId = j;
                }
            }
            if (maxDeltaQ > 0.0 && nei_comm_id[maxId] != cid){
                lv->nodes[ci].clsid      = nei_comm_id[maxId];
                lv->nodes[ci].sibling    = lv->nodes[nei_comm_id[maxId]].sibling;
                lv->nodes[nei_comm_id[maxId]].sibling = ci;
                lv->nodes[nei_comm_id[maxId]].count  += lv->nodes[ci].count;
                lv->nodes[nei_comm_id[maxId]].clstot += lv->nodes[ci].clstot;
                keepgoing = 1;
                need_stage_two = 1;
            }
        }
        if (keepgoing == 0){
            break;
        }
    }
    hash_free(in);
    in = NULL;
    free(nei_comm_id);
    free(nei_comm_weight);
    nei_comm_id = nei_comm_weight = NULL;
    return need_stage_two;
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

