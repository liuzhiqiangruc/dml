/* ========================================================
 *   Copyright (C) 2015 All rights reserved.
 *   
 *   filename : rbtree.c
 *   author   : liuzhiqiang01@baidu.com
 *              lizeming@baidu.com
 *   date     : 2015-02-02
 *   info     : implementation for rb tree
 * ======================================================== */

#include <stdio.h>
#include <stdlib.h>
#include "rbtree.h"

int default_cmp(int *a, int *b) {
    return (*a > *b) ? 1 : ((*a < *b) ? -1 : 0);
}

/* ---------------------------------------
 * brief  : rotate operation for node t
 * para t : node for operation
 * para d : rotate direction
 *          0 : left ---> right
 *          1 : right ---> left
 * return : new node after rotate
 * --------------------------------------- */
static void rotate(RBTree *t, RBNode *z, int d) {
    RBNode *s = z->child[d];
    RBNode *ss = s->child[1 ^ d];
    z->child[d] = ss;
    if (t->nil != ss) ss->p = z;
    s->p = z->p;
    if (z->p == t->nil) {
        t->root = s;
    }
    else {
        int sd = (z->p->child[0] == z) ? 0 : 1;
        s->p->child[sd] = s;
    }
    s->child[1 ^ d] = z;
    z->p = s;
}

/* ---------------------------------------
 * brief  : fix up for rb_insert
 * para t : rb Tree
 * para z : inserted node pointer
 * para d : z is left or right child
 * --------------------------------------- */
static void rb_insert_fix(RBTree *t, RBNode *z) {
    RBNode *y = NULL;    // sibling of parent
    int c = 0, d = 0;
    while (z->p->color == RB_RED) {
        d = z->p->child[0] == z ? 0 : 1;         // dir from p to z
        c = z->p->p->child[0] == z->p ? 0 : 1;   // dir from pp to p
        y = z->p->p->child[1 ^ c];
        if (y->color == RB_RED) {                /*     PP(BLACK)                PP(RED) <--Z      */
            z->p->color = y->color = RB_BLACK;   /*     /      \        I        /       \         */
            z->p->p->color = RB_RED;             /*    P(RED)   Y(RED)  ---->   P(BLACK) Y(BLACK)  */
            z = z->p->p;                         /*   /                        /                   */
        }                                        /*  S(RED) <--Z              S(RED)               */
        else {
            if (d != c) {                        
                z = z->p;                        /*     PP(B)            pp(R)                     */
                rotate(t, z, d);                 /*    /                /                          */
            }                                    /*   p(R)      --->   S(B)    --->    S(B)        */
            z->p->color = RB_BLACK;              /*    \              /               /   \        */
            z->p->p->color = RB_RED;             /*     S(R) <--Z    p(R)<-- Z   Z-->p(R)  pp(R)   */
            rotate(t, z->p->p, c);
        }
    }
    t->root->color = RB_BLACK;
}

/* ----------------------------------------
 * brief  : replace node z with node r
 * para t : rb tree
 * para z : rb node will be replaced
 * para r : rb node which will replace z
 * ---------------------------------------- */
static void rb_trans(RBTree *t, RBNode *z, RBNode *r) {
    if (z->p == t->nil) {
        t->root = r;
    }
    else {
        int sd = (z->p->child[0] == z) ? 0 : 1;
        z->p->child[sd] = r;
    }
    r->p = z->p;
}

/* --------------------------------------------
 * brief  : find min node from node z in tree t
 * para t : rb tree
 * para z : subroutine root
 * return : min node from z
 * -------------------------------------------- */
static RBNode *rb_min(RBTree *t, RBNode *z) {
    if (!z || !t || z == t->nil) {
        return NULL;
    }
    while (z->child[0] != t->nil) {
        z = z->child[0];
    }
    return z;
}

/* ----------------------------------------
 * brief : find the leftest or rightest value
 * para t: r b tree
 * para d: direction ,0: left, 1:right
 * return: pData pointer
 * ---------------------------------------- */
static void * found_l_r(RBTree * t, int d){
    if (!t) return NULL;
    RBNode *s = t->root;
    if (s == t->nil) return NULL;
    while(s->child[d] != t->nil){
        s = s->child[d];
    }
    return s->pData;
}

/* ----------------------------------------
 * brief  : find node whose data equals to pdata
 * para t : rb tree
 * para pdata : data pointer
 * return : NULL if not found 
 *          node pointer is found
 * ---------------------------------------- */
RBNode *node_find(RBTree *t, void *pdata) {
    if (!t || !pdata) return NULL;
    if (t->root == t->nil) return NULL;
    RBNode *s = t->root;
    while (s != t->nil) {
        int cp = t->cmp_fn(pdata, s->pData);
        if (cp == 0) return s;
        else if (cp > 0) s = s->child[1];
        else s = s->child[0];
    }
    return NULL;
}

/* --------------------------------------------
 * brief : delete sub tree from z
 * para t: red black tree
 * para z: root of subtree
 * -------------------------------------------- */
static void _del_node_t(RBTree * t, RBNode * z){
    if (!z || z == t->nil) return;
    if (z->child[0] != t->nil) _del_node_t(t, z->child[0]);
    if (z->child[1] != t->nil) _del_node_t(t, z->child[1]);
    if (t->free_fn){
        t->free_fn(z->pData);
    }
    free(z);
}

/* ---------------------------------------
 * brief  : fix up for rb_delete
 * para t : rb Tree
 * para z : moving node for delete operation
 * --------------------------------------- */
static void rb_delete_fix(RBTree *t, RBNode *z) {
    RBNode *w = NULL;     // for z's brother
    int d = 0;            // direction of z from z->p
    while (z != t->root && z->color == RB_BLACK) {
        d = (z->p->child[0] == z) ? 0 : 1;
        w = z->p->child[1 ^ d];  /* w can not be nil */    /* ---------------- case 1 ------------------- */
        if (w->color == RB_RED) {                          /*                                w(B)         */
            w->color = RB_BLACK;                           /*           p (B)               /             */
            z->p->color = RB_RED;                          /*          /  \       --->     p(R)           */
            rotate(t, z->p, 1 ^ d);                        /*    z--->s(B) w (R)          /   \           */
            w = z->p->child[1 ^ d];                        /*                        z-->s (B) sb(B)<--w  */
        }                                                  /* ---------------- case 2 ------------------- */
        if (w->child[d]->color == RB_BLACK &&              /*          p (R)           z--->p (R)         */
            w->child[1 ^ d]->color == RB_BLACK) {          /*         /  \                 /  \           */
            w->color = RB_RED;                             /*   z--->s(B) w (B)   --->    s(B) w (R)      */
            z = z->p;                                      /*            /  \                 /  \        */
        }                                                  /*          wl(B) wr(B)          wl(B) wr(B)   */
        else {                                             /* ---------------- case 3 ------------------- */
            if (w->child[1 ^ d]->color == RB_BLACK) {      /*           p(?)       z-->s(B)  w1(B)<--w    */
                w->child[d]->color = RB_BLACK;             /*          /  \                    \          */
                w->color = RB_RED;                         /*    z-->s(B) w (B)   --->          w(R)      */
                rotate(t, w, d);                           /*            /  \                    \        */
                w = z->p->child[1 ^ d];                    /*          wl(R) wr(B)                wr(B)   */
            }                                              /* ---------------- case 4 ------------------- */
            w->color = z->p->color;                        /*           p(?)                   w(?)       */
            z->p->color = RB_BLACK;                        /*          /  \                   /  \        */
            w->child[1 ^ d]->color = RB_BLACK;             /*    z-->s(B) w (B)   --->      p(B) wr (B)   */
            rotate(t, z->p, 1 ^ d);                        /*               \               /             */
            z = t->root;                                   /*                wr(R)         s(B)           */
        }                                                  /* ----------------- Done -------------------- */
    }
    z->color = RB_BLACK;
}


/* --------------------------------------
 * brief : create red black tree struct
 * para cmp_fn : compare function 
 * para free_fn: free function for pData
 * return : created red black tree 
 * -------------------------------------- */
RBTree *rb_create(CMP_FN cmp_fn, FREE_FN free_fn) {
    RBTree *rbt = (RBTree *) malloc(sizeof(RBTree));
    RBNode *nil = (RBNode *) malloc(sizeof(RBNode));
    if (!rbt || !nil) {
        return NULL;
    }

    nil->color = RB_BLACK;
    nil->pData = nil->p = nil->child[0] = nil->child[1] = NULL;
    rbt->root = rbt->nil = nil;

    rbt->cmp_fn = (CMP_FN)default_cmp;
    if (NULL != cmp_fn) {
        rbt->cmp_fn = cmp_fn;
    }
    rbt->free_fn = free_fn;

    return rbt;
}

/* --------------------------------------
 * brief : insert pdata into rb tree
 * para t: rb tree pointer
 * para pdata: data pointer
 * return : success 0, failed -1
 * -------------------------------------- */
int rb_insert(RBTree *t, void *pdata) {
    if (!t || !pdata) {
        return -1;
    }
    RBNode *z;

    z = (RBNode *) malloc(sizeof(RBNode));
    z->color = RB_RED;
    z->pData = pdata;
    z->child[0] = z->child[1] = t->nil;

    if (t->nil == t->root) {
        z->p = t->nil;
        t->root = z;
        t->root->color = RB_BLACK;
        return 0;
    }

    RBNode *s, *p;
    int c = 0;
    s = t->root;
    while (s != t->nil) {
        p = s;
        c = t->cmp_fn(pdata, s->pData) > 0 ? 1 : 0;
        s = s->child[c];
    }
    p->child[c] = z;
    z->p = p;

    if (p->color == RB_RED)
        rb_insert_fix(t, z);

    return 0;
}

/* --------------------------------------
 * brief : delete pdata from rb tree
 * para t: rb tree pointer
 * para pdata: data pointer
 * return : success 0, failed -1
 * -------------------------------------- */
int rb_delete(RBTree *t, void *pdata) {
    RBNode *z = node_find(t, pdata);
    if (!z) return -1;
    RBNode *y = z, *x = NULL;
    int color = y->color;
    if (z->child[0] == t->nil) {
        x = z->child[1];
        rb_trans(t, z, x);
    }
    else if (z->child[1] == t->nil) {
        x = z->child[0];
        rb_trans(t, z, x);
    }
    else {
        y = rb_min(t, z->child[1]);
        color = y->color;
        x = y->child[1];
        if (y->p == z) {
            x->p = y;
        }
        else {
            rb_trans(t, y, x);
            y->child[1] = z->child[1];
            y->child[1]->p = y;
        }
        rb_trans(t, z, y);
        y->child[0] = z->child[0];
        y->child[0]->p = y;
        y->color = z->color;
    }
    free(z);
    z = NULL;
    if (color == RB_BLACK) {
        rb_delete_fix(t, x);
    }
    return 0;
}


/* ----------------------------
 * brief  : found leftest value
 * return : pData pointer
 * ---------------------------- */
void * rb_find_left(RBTree * t){
    return found_l_r(t, 0);
}

/* ----------------------------
 * brief  : found rightest value
 * return : pData pointer
 * ---------------------------- */
void * rb_find_right(RBTree * t){
    return found_l_r(t, 1);
}

/* ----------------------------
 * brief : clear the tree 
 * para t: rb tree
 * ---------------------------- */
void rb_clear(RBTree * t){
    if (!t) return;
    _del_node_t(t, t->root);
    t->root = t->nil;
}

/* ----------------------------
 * brief : delete the tree 
 * para t: rb tree
 * ---------------------------- */
void rb_free(RBTree * t){
    if (!t) return ;
    rb_clear(t);
    free(t->nil);
    t->root = t->nil = NULL;
    free(t);
}

