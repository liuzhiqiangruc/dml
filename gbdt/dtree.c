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

struct _d_tree {
    int n;                          /* instance num in this node */
    int leaf;                       /* is leaf or not, 0 | 1     */
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

static int split(DTD   * ds
               , DTree * t
               , DTree * le
               , DTree * ri
               , DTree ** inst_nodes
               , double * g
               , double * h
               , double n_reg
               , double w_reg
               , int n)    {
    memset(le, 0, sizeof(DTree));
    memset(ri, 0, sizeof(DTree));
    int i, j, row, offs, cnt, attr, l_cnt;
    double l_sg, l_sh, r_sg, r_sh, l_val, val, gain, max_gain, aval, l_sg_b, l_sh_b;
    // split gain must be bigger than 0.0 !!!
    max_gain = 0.0;
    // this can be parallelized for scanning attributes !!!
    for (i = 0; i < ds->col; i++){
        offs = ds->cl[i];
        l_sg = l_sh = 0.0;
        r_sg = t->sg;
        r_sh = t->sh;
        l_val = 0.123456789;
        cnt = 0;
        for (j = 0; j < ds->l[i]; j++){
            row = ds->ids[offs + j];
            if (t == inst_nodes[row]){
                if (ds->bin == 0) {
                    val = ds->vals[offs + j];
                    if ((fabs(val - l_val) > 1e-6) && (fabs(l_val - 0.123456789) > 1e-6)) {
                        gain = 0.5 * (l_sg * l_sg / (l_sh + w_reg) \
                                    + r_sg * r_sg / (r_sh + w_reg) \
                                    - t->sg * t->sg / (t->sh + w_reg)) - n_reg;
                        if (gain > max_gain) {
                            max_gain = gain;
                            attr = i;
                            aval = (val + l_val) / 2.0;
                            l_sg_b = l_sg;
                            l_sh_b = l_sh;
                            l_cnt = cnt;
                        }
                    }
                    l_val = val;
                }
                l_sg += g[row]; l_sh += h[row];
                r_sg -= g[row]; r_sh -= h[row];
                cnt += 1;
            }
        }
        if (cnt > 0 && cnt < t->n){
            gain = 0.5 * (l_sg * l_sg / (l_sh + w_reg) \
                        + r_sg * r_sg / (r_sh + w_reg) \
                        - t->sg * t->sg / (t->sh + w_reg)) - n_reg;
            if (gain > max_gain){
                max_gain = gain;
                attr = i;
                if (ds->bin == 0){
                    aval = val;
                }
                l_sg_b = l_sg;
                l_sh_b = l_sh;
                l_cnt  = cnt;
            }
        }
    }
    if (max_gain > 0.0){
        t->attr      = attr;
        if (ds->bin == 0){
            t->attr_val  = aval;
        }
        t->child[0]  = le;
        t->child[1]  = ri;
        le->n        = l_cnt;
        ri->n        = t->n - l_cnt;
        le->sg       = l_sg_b;
        le->sh       = l_sh_b;
        le->wei      = -1.0 * le->sg / (le->sh + w_reg);
        le->loss     = -0.5 * le->sg * le->sg / (le->sh + w_reg) + n_reg;
        ri->sg       = t->sg - l_sg_b;
        ri->sh       = t->sh - l_sh_b;
        ri->wei      = -1.0 * ri->sg / (ri->sh + w_reg);
        ri->loss     = -0.5 * ri->sg * ri->sg / (ri->sh + w_reg) + n_reg;
        offs = ds->cl[attr];
        for (j = 0; j < ds->l[attr]; j++){
            row = ds->ids[offs + j];
            if (inst_nodes[row] == t){
                if (ds->bin == 1 || (ds->bin == 0 && ds->vals[offs + j] >= aval)) {
                    inst_nodes[row] = le;
                }
            }
        }
        for (i = 0; i < n; i++){
            if (inst_nodes[i] == t){
                inst_nodes[i] = ri;
            }
        }
        return 0;
    }
    return -1;
}

static void insert(DTree ** leafs, int l, DTree * t){
    int i;
    for (i = l; i > 0; i-= 1){
        if ((leafs[i - 1]->leaf == 1) || \
            (leafs[i - 1]->loss <= t->loss)){
            break;
        }
        leafs[i] = leafs[i - 1];
    }
    leafs[i] = t;
}

DTree * generate_dtree(DTD * ds, double * F, double * g, double * h, double n_reg, double w_reg, int n, int m){
    int i, l;
    if (m < 2)
        return NULL;
    // leaf nodes and instance node belonging
    DTree ** leaf_nodes = (DTree**)malloc(sizeof(DTree*) * m);
    DTree ** inst_nodes = (DTree**)malloc(sizeof(DTree*) * n);
    memset(leaf_nodes, 0, sizeof(DTree*) * m);
    memset(inst_nodes, 0, sizeof(DTree*) * n);
    // first leaf node : root of the tree;
    DTree * t = (DTree *)malloc(sizeof(DTree));
    memset(t, 0, sizeof(DTree));
    t->n = n;
    for (i = 0; i < n; i++){
        t->sg += g[i];
        t->sh += h[i];
        inst_nodes[i] = t;
    }
    t->wei  = -1.0 * t->sg / (t->sh + w_reg);
    t->loss = -0.5 * t->sg * t->sg / (t->sh + w_reg) + n_reg;
    // add first leaf node into leaf node sequence
    l = 0;
    leaf_nodes[l] = t;
    l += 1;
    // node split until leaf nodes sequence full
    // or all leaf node can not split;
    do {
        i = l - 1;
        DTree * p = leaf_nodes[i];
        if (p->leaf == 0){
            DTree * le = (DTree *)malloc(sizeof(DTree));
            DTree * ri = (DTree *)malloc(sizeof(DTree));
            if (split(ds, p, le, ri, inst_nodes, g, h, n_reg, w_reg, n) == 0){
                insert(leaf_nodes, i,     le);
                insert(leaf_nodes, i + 1, ri);
                l += 1;
            }
            else{
                free(le); le = NULL;
                free(ri); ri = NULL;
                p->leaf = 1;
                while (i > 0){
                    leaf_nodes[i] = leaf_nodes[i - 1];
                    i -= 1;
                }
                leaf_nodes[0] = p;
            }
        }
        else{
            break;
        }
    } while (l < m);
    // root can split
    if (t->leaf == 0){
        // update model F and 
        // set leaf nodes' leaf to 1
        for (i = 0; i < n; i++){
            F[i] = inst_nodes[i]->wei;
            inst_nodes[i]->leaf = 1;
        }
    }
    // root can not split
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
            fprintf(fp, "%d\t%d\t%s\t%.3f\t%.3f\t%.3f\t%d\t%d\n"         \
                                                      , ct->n            \
                                                      , ct->leaf         \
                                                      , id_map[ct->attr] \
                                                      , ct->attr_val     \
                                                      , ct->wei          \
                                                      , ct->loss         \
                                                      , c1, c2);

        }
        else{
            fprintf(fp, "%d\t%d\tNone\tNone\t%.3f\t%.3f\t%d\t%d\n"    \
                                                      , ct->n         \
                                                      , ct->leaf      \
                                                      , ct->wei       \
                                                      , ct->loss      \
                                                      , c1, c2);
        }
        i += 1;
    } while (i < l && l <= 1997);
    fclose(fp);
    free(ts);
    ts = NULL;
}
