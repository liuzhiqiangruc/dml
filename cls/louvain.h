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


typedef struct _node {
    unsigned int count;     // node number of current cluster
    unsigned int clsid;     // the upper cluster id
    unsigned int sibling;   // the next node which belong to the same upper cluster 
    double kin;       // current node in weight
    double kout;      // current node out weight
    double clstot;    // nodes which belong to the same cluster have the same clstot;
} Node;

typedef struct _edge {
    unsigned int left;
    unsigned int right;
    unsigned int next;
    double weight;
} Edge;



#endif //LOUVAIN_H
