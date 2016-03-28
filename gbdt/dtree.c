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

static void update_model(double w, unsigned long * bit_map, int len, double * F, int n) {
    int i, offs, ind;
    unsigned long v   = 0UL;
    unsigned char uc  = 0;
    unsigned char luc = 0;
    // Log Dict
    int LogD[129] = {0};
    LogD[1]       =  0;
    LogD[2]       =  1;
    LogD[4]       =  2;
    LogD[8]       =  3;
    LogD[16]      =  4;
    LogD[32]      =  5;
    LogD[64]      =  6;
    LogD[128]     =  7;
    for (i = 0; i < len; i++){
        v = bit_map[i];
        offs = 0;
        while (v > 0UL){
            uc = v & 0xff;
            while (uc > 0){
                luc = uc & (-uc);
                ind = LogD[luc] + offs + (i << 6);
                if (ind >= n){
                    return;
                }
                F[ind] += w;
                uc -= luc;
            }
            v = v >> 8;
            offs += 8;
        }
    }
}

static void scan_tree(DTD * ts, DTree * t, unsigned long * bit_map, int len, double * F, int n, int m){
    int id, rowid, l_c, r_c;
    unsigned long v;
    unsigned long * l_bit = NULL;
    unsigned long * r_bit = NULL;
    if (t->leaf == 0){
        l_bit = (unsigned long *)malloc(len << 3);
        r_bit = (unsigned long *)malloc(len << 3);
        memset(l_bit, 0, len << 3);
        memmove(r_bit, bit_map, len << 3);
        l_c = 0;
        r_c = m;
        for (int i = 0; i < ts->l[t->attr]; i++){
            id = i + ts->cl[t->attr];
            rowid = ts->ids[id];
            v = bit_map[rowid >> 6];
            if ((v  & (1UL << (rowid & 63))) > 0){
                if (ts->bin == 1 || (ts->bin == 0 && ts->vals[id] >= t->attr_val)){
                    l_bit[rowid >> 6] |= (1UL << (rowid & 63));
                    r_bit[rowid >> 6] ^= (1UL << (rowid & 63));
                    l_c += 1;
                    r_c -= 1;
                }
            }
        }
        if (l_c > 0){
            scan_tree(ts, t->child[0], l_bit, len, F, n, l_c);
        }
        free(l_bit); l_bit = NULL;
        if (r_c > 0){
            scan_tree(ts, t->child[1], r_bit, len, F, n, r_c);
        }
        free(r_bit); r_bit = NULL;
    }
    else{
        update_model(t->wei, bit_map, len, F, n);
    }
}

static int split(DTD * ds
               , DTree * t
               , DTree * le
               , DTree * ri
               , unsigned long * bit_map
               , unsigned long * l_bit
               , unsigned long * r_bit
               , int len
               , double * g
               , double * h
               , int n) {
    memset(le, 0, sizeof(DTree));
    memset(ri, 0, sizeof(DTree));
    int i, j, row, offs, cnt, attr, l_cnt;
    double l_sg, l_sh, r_sg, r_sh, l_val, val, gain, max_gain, aval, l_sg_b, l_sh_b;
    unsigned long * ll_bit = (unsigned long *)malloc(len << 3);
    unsigned long * rr_bit = (unsigned long *)malloc(len << 3);
    // split gain must be bigger than 0.0 !!!
    max_gain = 0.0;
    // this can be parallelized for scanning attributes !!!
    for (i = 0; i < ds->col; i++){
        offs = ds->cl[i];
        memset (ll_bit, 0,       len << 3);
        memmove(rr_bit, bit_map, len << 3);
        l_sg = l_sh = 0.0;
        r_sg = t->sg;
        r_sh = t->sh;
        l_val = 0.123456789;
        cnt = 0;
        for (j = 0; j < ds->l[i]; j++){
            row = ds->ids[offs + j];
            if (ds->bin == 0){
                val = ds->vals[offs + j];
            }
            if ((ds->bin == 0) && (fabs(val - l_val) > 1e-6) && (fabs(l_val - 0.123456789) > 1e-6)){
                gain = 0.5 * (l_sg * l_sg / l_sh + r_sg * r_sg / r_sh - t->sg * t->sg / t->sh);
                if (gain > max_gain){
                    max_gain = gain;
                    attr = i;
                    aval = (val + l_val) / 2.0;
                    l_sg_b = l_sg;
                    l_sh_b = l_sh;
                    l_cnt  = cnt;
                    memmove(l_bit, ll_bit, len << 3);
                    memmove(r_bit, rr_bit, len << 3);
                }
            }
            unsigned long f = bit_map[row>>6];
            if ((f & (1UL << (row & 63)))> 0){
                ll_bit[row >> 6] |= (1UL << (row & 63));
                rr_bit[row >> 6] ^= (1UL << (row & 63));
                l_sg += g[row]; l_sh += h[row];
                r_sg -= g[row]; r_sh -= h[row];
                cnt += 1;
            }
            if (ds->bin == 0){
                l_val = val;
            }
        }
        if (cnt > 0 && cnt < t->n){
            gain = 0.5 * (l_sg * l_sg / l_sh + r_sg * r_sg / r_sh - t->sg * t->sg / t->sh);
            if (gain > max_gain){
                max_gain = gain;
                attr = i;
                if (ds->bin == 0){
                    aval = val;
                }
                l_sg_b = l_sg;
                l_sh_b = l_sh;
                l_cnt  = cnt;
                memmove(l_bit, ll_bit, len << 3);
                memmove(r_bit, rr_bit, len << 3);
            }
        }
    }
    free(ll_bit); ll_bit = NULL;
    free(rr_bit); rr_bit = NULL;
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
        le->wei      = -1.0 * le->sg / le->sh;
        le->loss     = -0.5 * le->sg * le->sg / le->sh;
        ri->sg       = t->sg - l_sg_b;
        ri->sh       = t->sh - l_sh_b;
        ri->wei      = -1.0 * ri->sg / ri->sh;
        ri->loss     = -0.5 * ri->sg * ri->sg / ri->sh;
        return 0;
    }
    else{
        return -1;
    }
}

static void insert(DTree ** leafs, unsigned long ** bmaps, int l, DTree * t, unsigned long * bmap){
    int i;
    for (i = l; i > 0; i-= 1){
        if ((leafs[i - 1]->leaf == 1) || \
            (leafs[i - 1]->loss <= t->loss)){
            break;
        }
        leafs[i] = leafs[i - 1];
        bmaps[i] = bmaps[i - 1];
    }
    leafs[i] = t;
    bmaps[i] = bmap;
}

DTree * generate_dtree(DTD * ds, double * F, double * g, double * h, int n, int m){
    int i, l;
    if (m < 2)
        return NULL;
    // len of bit map
    int len = n >> 6;
    if ((n & 63) > 0){
        len += 1;
    }
    // leaf nodes sequences
    DTree ** leaf_nodes = (DTree**)malloc(sizeof(DTree*) * m);
    unsigned long ** bit_maps = (unsigned long **)malloc(sizeof(unsigned long*) * m);
    memset(leaf_nodes, 0, sizeof(DTree*) * m);
    memset(bit_maps,   0, sizeof(unsigned long *) * m);
    // first leaf node : root of the tree;
    DTree * t = (DTree *)malloc(sizeof(DTree));
    memset(t, 0, sizeof(DTree));
    unsigned long * bit_map = (unsigned long *)malloc(len << 3);
    memset(bit_map, -1, len << 3);
    t->n = n;
    for (i = 0; i < n; i++){
        t->sg += g[i];
        t->sh += h[i];
    }
    t->wei = -1.0 * t->sg / t->sh;
    t->loss = -0.5 * t->sg * t->sg / t->sh;
    // add first leaf node into leaf node sequence
    l = 0;
    leaf_nodes[l] = t;
    bit_maps[l] = bit_map;
    l += 1;
    // node split until leaf nodes sequence full
    // or all leaf node can not split;
    do {
        i = l - 1;
        DTree * p = leaf_nodes[i];
        if (p->leaf == 0){
            bit_map = bit_maps[i];
            bit_maps[i] = NULL;
            unsigned long * l_bit = (unsigned long *)malloc(len << 3);
            unsigned long * r_bit = (unsigned long *)malloc(len << 3);
            DTree * le = (DTree *)malloc(sizeof(DTree));
            DTree * ri = (DTree *)malloc(sizeof(DTree));
            if (split(ds, p, le, ri, bit_map, l_bit, r_bit, len, g, h, n) == 0){
                insert(leaf_nodes, bit_maps, i,     le, l_bit);
                insert(leaf_nodes, bit_maps, i + 1, ri, r_bit);
                free(bit_map); bit_map = NULL;
                l += 1;
            }
            else{
                free(le); le = NULL;
                free(ri); ri = NULL;
                free(l_bit); l_bit = NULL;
                free(r_bit); r_bit = NULL;
                p->leaf = 1;
                while (i > 0){
                    leaf_nodes[i] = leaf_nodes[i - 1];
                    bit_maps[i] = bit_maps[i - 1];
                    i -= 1;
                }
                leaf_nodes[0] = p;
                bit_maps[0] = bit_map;
            }
        }
        else{
            break;
        }
    } while (l < m);
    // set leaf nodes' leaf == 1
    // update model F;
    // free bit_maps
    if (t->leaf == 0){
        for (i = 0; i < l; i++){
            leaf_nodes[i]->leaf = 1;
            update_model(leaf_nodes[i]->wei, bit_maps[i], len, F, n);
            free(bit_maps[i]);
            bit_maps[i] = NULL;
        }
    }
    else{
        free(bit_maps[0]); bit_maps[0] = NULL;
        free(t); t = NULL;
    }
    free(leaf_nodes);    leaf_nodes = NULL;
    free(bit_maps);        bit_maps = NULL;
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
    int len;
    len = n >> 6;
    if ((n & 63) > 0){
        len += 1;
    }
    unsigned long * bit_map = (unsigned long *)malloc(len << 3);
    memset(bit_map, -1, len << 3);
    scan_tree(ts, t, bit_map, len, F, n, n);
    free(bit_map);   bit_map = NULL;
    return F;
}

void save_dtree(DTree * t, char * out_file){
    if (!t){
        return;
    }
    FILE * fp = NULL;
    if (NULL == (fp = fopen(out_file, "w"))){
        fprintf(stderr, "save out file \"%s\"\n", out_file);
        return;
    }

    fclose(fp);
}


