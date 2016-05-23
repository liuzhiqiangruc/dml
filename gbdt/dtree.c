/* ========================================================
 *   Copyright (C) 2016 All rights reserved.
 *
 *   filename : dtree.c
 *   author   : liuzhiqiangruc@126.com
 *   date     : 2016-02-26
 *   info     : implementation for decision_tree model
 * ======================================================== */
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include "dtree.h"

#define  DBL_MAX 1.1e10

struct _d_tree {
    int n;                          /* instance num in this node */
    int leaf;                       /* is leaf or not, 0 | 1     */
    int depth;                      /* depth of this node        */
    struct _d_tree * child[2];      /* splited children nodes    */
    int attr;                       /* if not leaf, split attr   */
    double attr_val;                /* split val of this attr    */
    double sg;                      /* sum of 1-gradient         */
    double sh;                      /* sum of 2-gradient         */
    double wei;                     /* additive model value      */
    double loss;                    /* loss value of this node   */
};

static void scan_tree(DTD * ts, DTree * t, DTree ** inst_nodes, int n, int m){
    int i, id, rowid, l_c, r_c;
    l_c = 0;
    r_c = m;
    for (i = 0; i < ts->l[t->attr]; i++){
        id = i + ts->cl[t->attr];
        rowid = ts->ids[id];
        if (inst_nodes[rowid] == t) {
            if (ts->bin == 1 || (ts->bin == 0 && ts->vals[id] >= t->attr_val)){
                l_c += 1;
                r_c -= 1;
                inst_nodes[rowid] = t->child[0];
            }
        }
    }
    for (i = 0; i < n; i++){
        if (inst_nodes[i] == t){
            inst_nodes[i] = t->child[1];
        }
    }
    if (l_c > 0 && t->child[0]->leaf == 0) {
        scan_tree(ts, t->child[0], inst_nodes, n, l_c);
    }
    if (r_c > 0 && t->child[1]->leaf == 0) {
        scan_tree(ts, t->child[1], inst_nodes, n, r_c);
    }
}

static void init_child(DTree * t){
    DTree * le = (DTree *)malloc(sizeof(DTree));
    DTree * ri = (DTree *)malloc(sizeof(DTree));
    memset(le, 0, sizeof(DTree));
    memset(ri, 0, sizeof(DTree));
    t->leaf     = 0;
    t->child[0] = le;
    t->child[1] = ri;
    ri->n       = t->n;
    ri->sg      = t->sg;
    ri->sh      = t->sh;
    ri->wei     = t->wei;
    ri->loss    = t->loss;
    le->leaf    = ri->leaf  = 1;
    le->depth   = ri->depth = t->depth + 1;
}

static void update_child(DTree * t, int k, int lc, double lsg, double lsh, double nr, double wr, double v, double lv){
    double l_loss = -0.5 * lsg * lsg / (lsh + wr) + nr;
    double r_loss = -0.5 * (t->sg - lsg) * (t->sg - lsg) / (t->sh - lsh + wr) + nr;
    double gain = t->loss - l_loss - r_loss;
    if (gain > t->loss - t->child[0]->loss - t->child[1]->loss && gain > 0.0){
        t->attr = k;
        t->attr_val = (v + lv ) / 2.0;
        t->child[0]->sg = lsg;    t->child[1]->sg = t->sg - lsg;
        t->child[0]->sh = lsh;    t->child[1]->sh = t->sh - lsh;
        t->child[0]->n  = lc;     t->child[1]->n  = t->n  - lc;
        t->child[0]->loss = l_loss; 
        t->child[1]->loss = r_loss;
        t->child[0]->wei = -1.0 * lsg / (lsh + wr);
        t->child[1]->wei = -1.0 * (t->sg - lsg) / (t->sh - lsh + wr);
    }
}

static int tree_grow(DTD * ds
                , DTree ** leaf_nodes
                , int    * inst_nodes
                , double * g
                , double * h
                , double nr
                , double wr
                , int l
                , int n
                , int d
                , int m){
    int i, j, k, o, r, lc;
    double v = 0.0, lv = DBL_MAX, l_sg, l_sh, gain;
    DTree * t = NULL;
    for (i = 0; i < l; i++){
        t = leaf_nodes[i];
        if (1 == t->leaf && t->depth < d){
            init_child(t);
            for (j = 0; j < ds->col; j++){
                l_sg = l_sh = 0.0;
                lv = DBL_MAX;
                lc = 0;
                o = ds->cl[j];
                for (k = 0; k < ds->l[j]; k++){
                    r = ds->ids[o + k];
                    if (inst_nodes[r] == i){
                        if (0 == ds->bin){
                            v = ds->vals[o + k];
                            if (v < lv && lv < DBL_MAX){
                                update_child(t, j, lc, l_sg, l_sh, nr, wr, v, lv);
                            }
                            lv = v;
                        }
                        lc += 1;
                        l_sg += g[r];
                        l_sh += h[r];
                    }
                }
                if (lc < t->n && lc > 0){
                    update_child(t, j, lc, l_sg, l_sh, nr, wr, v, 1 == ds->bin ? 1.0 : lv);
                }
            }
        }
    }
    v = 0.0;
    k = -1;
    for (i = 0; i < l; i++){
        t = leaf_nodes[i];
        if (0 == t->leaf){
            gain = t->loss - t->child[0]->loss - t->child[1]->loss;
            if (gain > v){
                v = gain;
                k = i;
            }
        }
    }
    return k;
}

static DTree * init_root(double * g, double * h, int n, double nr, double wr){
    int i;
    DTree * t = (DTree *)malloc(sizeof(DTree));
    memset(t, 0, sizeof(DTree));
    t->n = n;
    t->depth = 0;
    t->leaf = 1;
    for (i = 0; i < n; i++){
        t->sg += g[i];
        t->sh += h[i];
    }
    t->wei  = -1.0 * t->sg / (t->sh + wr);
    t->loss = -0.5 * t->sg * t->sg / (t->sh + wr) + nr;
    return t;
}

DTree * generate_dtree(DTD * ds      /* dataset for build tree */
                     , double * F    /* current f vector       */
                     , double * g    /* current gradient vec   */
                     , double * h    /* current hessian  vec   */
                     , double nr     /* node regulizatin       */
                     , double wr     /* weight regulization    */
                     , int n         /* number of instances    */
                     , int d         /* max depth of tree      */
                     , int m){       /* max leaf nodes         */
    int i, k, o, l;
    if (m < 2)
        return NULL;
    DTree ** leaf_nodes = (DTree**)malloc(sizeof(DTree*) * m);
    memset(leaf_nodes, 0, sizeof(DTree*) * m);
    int * inst_nodes = (int*)malloc(sizeof(int) * n);
    memset(inst_nodes, 0, sizeof(int) * n);
    DTree * t = init_root(g, h, n, nr, wr);
    l = 0;
    for (i = 0; i < n; i++){
        inst_nodes[i] = l;
    }
    leaf_nodes[l++] = t;
    while (l < m){
        if(-1 == (k = tree_grow(ds, leaf_nodes, inst_nodes, g, h, nr, wr, l, n, d, m))){
            break;
        }
        DTree * tmp = leaf_nodes[k];
        o = ds->cl[tmp->attr];
        for (i = 0; i < ds->l[tmp->attr]; i++){
            if (inst_nodes[ds->ids[o + i]] == k){
                if ((1 == ds->bin) || (0 == ds->bin && ds->vals[o + i] >= tmp->attr_val)){
                    inst_nodes[ds->ids[o + i]] = l;
                }
            }
        }
        leaf_nodes[l++] = tmp->child[0];
        leaf_nodes[k] = tmp->child[1];
    }
    for (i = 0; i < l; i++){
        if (0 == leaf_nodes[i]->leaf){
            leaf_nodes[i]->leaf = 1;
            free(leaf_nodes[i]->child[0]);
            free(leaf_nodes[i]->child[1]);
            leaf_nodes[i]->child[0] = NULL;
            leaf_nodes[i]->child[1] = NULL;
        }
    }
    if (t->leaf == 0){
        for (i = 0; i < n; i++){
            F[i] = leaf_nodes[inst_nodes[i]]->wei;
        }
    }
    else{
        free(t); t = NULL;
    }
    free(leaf_nodes);    leaf_nodes = NULL;
    free(inst_nodes);    inst_nodes = NULL;
    return t;
}

void free_dtree(DTree * t){
    if (t){
        if(t->child[0]){
            free_dtree(t->child[0]);
            t->child[0] = NULL;
        }
        if (t->child[1]){
            free_dtree(t->child[1]);
            t->child[1] = NULL;
        }
        free(t);
    }
}

double * eval_tree(DTD * ts, DTree * t, double * F, int n){
    int i;
    DTree ** inst_nodes = NULL;
    if (ts && F && t && t->leaf == 0 && n > 0) {
        inst_nodes = (DTree**)malloc(sizeof(DTree *) * n);
        for (i = 0; i < n; i++){
            inst_nodes[i] = t;
        }
        scan_tree(ts, t, inst_nodes, n, n);
        for (i = 0; i < n; i++){
            F[i] = inst_nodes[i]->wei;
        }
        free(inst_nodes); inst_nodes = NULL;
    }
    return F;
}

void save_dtree(DTree * t, char * out_file, char (*id_map)[FKL]){
    if (!t){
        return;
    }
    FILE * fp = NULL;
    if (NULL == (fp = fopen(out_file, "w"))){
        fprintf(stderr, "save out file \"%s\"\n", out_file);
        return;
    }
    // max 1000 leaf_nodes and 999 non_leaf nodes in tree
    DTree ** ts = (DTree **)malloc(sizeof(void *) * 1999);
    memset(ts, 0, sizeof(void*) * 1999);
    int i, l, c1, c2;
    i = l = 0;
    ts[l++] = t;
    fprintf(fp, "n\tleaf\tAttr\tAttr_val\tNode_wei\tNode_loss\tleft\tright\n");
    do {
        DTree * ct = ts[i];
        c1 = c2 = 0;
        // if is not leaf, push two children into ts
        if (ct->leaf == 0){
            c1 = l; ts[l++] = ct->child[0];
            c2 = l; ts[l++] = ct->child[1];
            fprintf(fp, "%d\t%d\t%s\t%.3f\t%.3f\t%.3f\t%d\t%d\n"          \
                      , ct->n,   ct->leaf, id_map[ct->attr], ct->attr_val \
                      , ct->wei, ct->loss, c1, c2);
        }
        else{
            fprintf(fp, "%d\t%d\tNone\tNone\t%.3f\t%.3f\t%d\t%d\n"    \
                      , ct->n, ct->leaf, ct->wei, ct->loss, c1, c2);
        }
        i += 1;
    } while (i < l && l <= 1997);
    fclose(fp);
    free(ts);
    ts = NULL;
}
