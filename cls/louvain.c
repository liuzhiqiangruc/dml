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
    int    count;        // node number of current cluster int    clsid;
    int    clsid;        // the upper cluster id
    int    next;         // the next node which belong to the same upper cluster 
    int    prev;         // the prev node which belong to the same upper cluster
    int    first;        // the first child of current community
    int    eindex;       // first neighbor index 
    double kin;          // current node in weight
    double kout;         // current node out weight
    double clskin;       // the kin value for new community
    double clstot;       // nodes which belong to the same cluster have the same clstot;
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
    int  nlen;
    int  olen;
    int  * cindex;
    double sumw;
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

static void INIT_NODE(Louvain * lv, int I, double weight){
    if (lv->cindex[I] == -1){         
        lv->cindex[I]         = I;            
        lv->nodes[I].count    = 1;       
        lv->nodes[I].kin      = 0;         
        lv->nodes[I].clskin   = 0;
        lv->nodes[I].clsid    = I;       
        lv->nodes[I].first    = -1;
        lv->nodes[I].prev     = -1;
        lv->nodes[I].next     = -1;
    }                                 
    lv->nodes[I].kout   += weight;
    lv->nodes[I].clstot += weight;
}

static void LINKEDGE(Louvain * lv, int l, int r, int ei, double weight){
    lv->edges[ei].left = l;                         
    lv->edges[ei].right = r;                        
    lv->edges[ei].weight = weight;                     
    lv->edges[ei].next = lv->nodes[l].eindex;       
    lv->nodes[l].eindex = ei;                       
}

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
    double weight = 1.0;
    while (NULL != fgets(buffer, PLEN, fp)){
        string = trim(buffer, 3);
        hash_add(hs, strsep(&string, "\t"));
        hash_add(hs, strsep(&string, "\t"));
        l += 1;
    }
    lv->clen = hash_cnt(hs);
    lv->elen = l * 2;
    lv->nlen = lv->clen;
    lv->olen = lv->elen;
    malloc_louvain(lv);
    rewind(fp);
    while (NULL != fgets(buffer, PLEN, fp)){
        string = trim(buffer, 3);
        l = hash_find(hs, strsep(&string, "\t"));
        r = hash_find(hs, strsep(&string, "\t"));
        weight = atof(strsep(&string, "\t"));
        lv->sumw += weight;   // left ---- > right ===== right ------> left
        INIT_NODE(lv, l, weight);
        INIT_NODE(lv, r, weight);
        LINKEDGE(lv, l, r, ei++, weight);
        LINKEDGE(lv, r, l, ei++, weight);
    }
    fclose(fp);
    return lv;
}

static void add_node_to_comm(Louvain * lv, int id, int cid, double weight){
    lv->nodes[id].clsid   = cid;
    lv->nodes[id].next    = lv->nodes[cid].next;
    lv->nodes[cid].next   = id;
    lv->nodes[id].prev    = cid;
    if (lv->nodes[id].next != -1){
        lv->nodes[lv->nodes[id].next].prev = id;
    }
    lv->nodes[cid].count  += lv->nodes[id].count;
    lv->nodes[cid].clstot += lv->nodes[id].clstot;
    lv->nodes[cid].clskin += lv->nodes[id].kin + 2 * weight;
}

static void remove_node_from_comm(Louvain * lv, int id, double weight){
    int cid = lv->nodes[id].clsid;
    int prev, next;
    if (cid != id){
        prev = lv->nodes[id].prev;
        next = lv->nodes[id].next;
        lv->nodes[prev].next = next;
        if (next != -1){
            lv->nodes[next].prev = prev;
        }
        lv->nodes[cid].count  -= lv->nodes[id].count;
        lv->nodes[cid].clstot -= lv->nodes[id].clstot;
        lv->nodes[cid].clskin -= lv->nodes[id].kin + 2 * weight;
    }
    else{
        next = lv->nodes[id].next; // the new center of the community
        cid = next;                // cid , new center 
        if (-1 != next){
            lv->nodes[next].prev = -1;
            lv->nodes[next].clsid = next;
            while (-1 != (next = lv->nodes[next].next)){
                lv->nodes[cid].count += lv->nodes[next].count;
                lv->nodes[next].clsid = cid;
            }
            lv->nodes[cid].clstot = lv->nodes[id].clstot - lv->nodes[id].kin - lv->nodes[id].kout;
            lv->nodes[cid].clskin = lv->nodes[id].clskin - lv->nodes[id].kin - 2 * weight;   
            lv->nodes[id].count  -= lv->nodes[cid].count;
            lv->nodes[id].clskin  = lv->nodes[id].kin;
            lv->nodes[id].clstot -= lv->nodes[cid].clstot;
        }
    }
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
    double kv, wei, cwei;
    double deltaQ, maxDeltaQ;
    double * nei_comm_weight = NULL;
    Hash * in = hash_create(1 << 16, INT);
    hsize = hash_size(in);
    hold_nei_comm(&nei_comm_id, &nei_comm_weight, NULL, hsize);
    while (1){
        keepgoing = 0;
        fprintf(stderr, "-----------------------\n");
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
            cwei = 0.0;
            id = hash_cnt(in);
            for (j = 0; j < id; j++){
                if (cid == nei_comm_id[j]){
                    deltaQ = nei_comm_weight[j] - kv * (lv->nodes[nei_comm_id[j]].clstot - kv) / lv->sumw;
                    cwei   = nei_comm_weight[j];
                }
                else{
                    deltaQ = nei_comm_weight[j] - kv * lv->nodes[nei_comm_id[j]].clstot / lv->sumw;
                }
                if (deltaQ > maxDeltaQ){
                    maxDeltaQ = deltaQ;
                    maxId = j;
                }
            }
            if (maxDeltaQ > 0.0 && nei_comm_id[maxId] != cid){
                fprintf(stderr, "change happen, with maxDeltaQ: %.6f\n", maxDeltaQ);
                fprintf(stderr, "    remove %d from cls: %d\n", ci, lv->nodes[ci].clsid);
                remove_node_from_comm(lv, ci, cwei);
                fprintf(stderr, "    add    %d to   cls: %d\n", ci, nei_comm_id[maxId]);
                add_node_to_comm(lv, ci, nei_comm_id[maxId], nei_comm_weight[maxId]);
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
    nei_comm_id = NULL;
    nei_comm_weight = NULL;
    return need_stage_two;
}

static void second_stage(Louvain * lv){
    int i, ci, next, first, tclen = 0;
    int l, r, telen = 0, lcid, rcid;
    double w;
    for (i = 0; i < lv->clen; i++){
        ci = lv->cindex[i];
        if (lv->nodes[ci].clsid == ci){
            lv->cindex[tclen++] = ci;
            next = lv->nodes[ci].next;
            first = lv->nodes[ci].first;
            if (first != -1){
                while (-1 != (lv->nodes[first].next)){
                    first = lv->nodes[first].next;
                }
                lv->nodes[first].next = next;
            }
            else{
                lv->nodes[ci].first = next;
            }
            if (next != -1){
                lv->nodes[next].prev = first;
            }
            lv->nodes[ci].next = -1;
            lv->nodes[ci].prev = -1;
        }
    }
    lv->clen = tclen;
    for (i = 0; i < lv->clen; i++){
        ci = lv->cindex[i];
        lv->nodes[ci].kin  = lv->nodes[ci].clskin;
        lv->nodes[ci].kout = lv->nodes[ci].clstot - lv->nodes[ci].kin;
        lv->nodes[ci].eindex = -1;
    }
    for (i = 0; i < lv->elen; i++){
        l = lv->edges[i].left;
        r = lv->edges[i].right;
        w = lv->edges[i].weight;
        lcid = lv->nodes[l].clsid;
        rcid = lv->nodes[r].clsid;
        if (lcid != rcid){
            lv->edges[telen].left = lcid;
            lv->edges[telen].right = rcid;
            lv->edges[telen].weight = w;
            lv->edges[telen].next = lv->nodes[lcid].eindex;
            lv->nodes[lcid].eindex = telen++;
        }
    }
    lv->elen = telen;
}

int learn_louvain(Louvain * lv){
    while (first_stage(lv)){
        for (int i = 0; i < lv->clen; i++){
            fprintf(stderr, "%d\t%d\t%d\t%d\t%d\t%d\t%f\t%f\t%f\t%f\n", lv->nodes[i].count
                     , lv->nodes[i].clsid
                     , lv->nodes[i].next
                     , lv->nodes[i].prev
                     , lv->nodes[i].first
                     , lv->nodes[i].eindex
                     , lv->nodes[i].kin
                     , lv->nodes[i].kout
                     , lv->nodes[i].clskin
                     , lv->nodes[i].clstot);
        }
        second_stage(lv);
        fprintf(stderr, "--------\n");
        for (int i = 0; i < lv->clen; i++){
            int j = lv->cindex[i];
            fprintf(stderr, "%d\t%d\t%d\t%d\t%d\t%d\t%f\t%f\t%f\t%f\n"
                     , lv->nodes[j].count
                     , lv->nodes[j].clsid
                     , lv->nodes[j].next
                     , lv->nodes[j].prev
                     , lv->nodes[j].first
                     , lv->nodes[j].eindex
                     , lv->nodes[j].kin
                     , lv->nodes[j].kout
                     , lv->nodes[j].clskin
                     , lv->nodes[j].clstot);
        }
        fprintf(stderr,"===================\n");
    }
    return 0;
}

void save_louvain(Louvain * lv){
}

void free_louvain(Louvain * lv){
}

