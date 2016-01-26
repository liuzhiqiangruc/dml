/* ========================================================
 *   Copyright (C) 2015 All rights reserved.
 *   
 *   filename : rbtree.c
 *   author   : liuzhiqiangruc@126.com
 *   date     : 2015-12-29
 *   info     : reb black tree implementation using
 *              memory pool 
 * ======================================================== */

#include <stdlib.h>
#include <string.h>
#include "rbtree.h"

#define RB_SPACE 0x10000

// rbcolor 
typedef enum {RB_RED, RB_BLACK} RB_NODE_COLOR;

// rbnode
typedef struct _RBNode{
    void * pData;
    RB_NODE_COLOR color;
    struct _RBNode * child[2];
    struct _RBNode * p;
} RBNode;

// rb linked array node
typedef union _la_node{
    RBNode rbnode;
    union _la_node * next;
} RBLANode;

// rb tree 
struct _RBTree{
    int rb_capacity;
    int rb_count;
    RBNode   * root;
    RBNode   * nil;
    RBLANode * rb_node_space;
    RBLANode * p;
    CMP_FN  cmp_fn;
    FREE_FN free_fn;
};

// default compare function
static int default_cmp(int *a, int *b) {
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
 * brief  : max min node from node z
 * para t : rb tree
 * para z : subroutine root
 * return : min max node from z
 * -------------------------------------------- */
static RBNode * rb_mm(RBTree * t, RBNode * z, int d){
    if (!z || !t || z == t->nil){
        return NULL;
    }
    while (z->child[d] != t->nil){
        z = z->child[d];
    }
    return z;
}

/* --------------------------------------------
 * brief  : find min node from node z in tree t
 * para t : rb tree
 * para z : subroutine root
 * return : min node from z
 * -------------------------------------------- */
static RBNode * rb_min(RBTree *t, RBNode *z) {
    return rb_mm(t, z, 0);
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
        if (w->child[d]->color == RB_BLACK &&              /*          p (?)           z--->p (?)         */
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

/* ----------------------------------------
 * brief  : find node whose data equals to pdata
 * para t : rb tree
 * para pdata : data pointer
 * return : NULL if not found 
 *          node pointer is found
 * ---------------------------------------- */
static RBNode * rb_find(RBTree *t, void *pdata) {
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
 * brief : delete data space of sub tree from z
 *         not delete the node space!!!
 *         this is called only if :
 *                     t->free_fn is not null!!
 * para t: red black tree
 * para z: root of subtree
 * -------------------------------------------- */
static void _del_node_t(RBTree * t, RBNode * z){
    if (!z || z == t->nil) return;
    if (z->child[0] != t->nil) _del_node_t(t, z->child[0]);
    if (z->child[1] != t->nil) _del_node_t(t, z->child[1]);
    t->free_fn(z->pData);
}

/* --------------------------------------------------
 * brief   : re local the RBNode children and parent
 * para t  : rb tree
 * para z  : current node 
 * para ns : new node space
 * -------------------------------------------------- */
static void _relocal_node_t(RBTree * t, RBNode * z, RBLANode * ns){
    if (!z || z == t->nil){
        return ;
    }
    if (z->p != t->nil) z->p = (RBNode*)(ns + ((RBLANode*)z->p - t->rb_node_space));
    if (z->child[0] != t->nil) {
        z->child[0] = (RBNode*)(ns + ((RBLANode*)z->child[0] - t->rb_node_space));
        _relocal_node_t(t, z->child[0], ns);
    }
    if (z->child[1] != t->nil){
        z->child[1] = (RBNode*)(ns + ((RBLANode*)z->child[1] - t->rb_node_space));
        _relocal_node_t(t, z->child[1], ns);
    }
}

/* ---------------------------------------------
 * brief   : re local the free node space
 * para t  : rb tree
 * para ns : new node space
 * --------------------------------------------- */
static void _relocal_node_free(RBTree * t, RBLANode * ns){
    RBLANode * p = ns + (t->p - t->rb_node_space);
    while (p && p->next){
        p->next = ns + (p->next - t->rb_node_space);
        p = p->next;
    }
    for (int i = 0; i < RB_SPACE; i++){
        p->next = ns + t->rb_capacity + i;
        p = p->next;
    }
    p->next = NULL;
}

/* --------------------------------------------------
 * brief  : extend the tree space by RB_SPACE
 * para t : rb tree
 * -------------------------------------------------- */
static int _extend_node_space(RBTree * t){
    int new_rb_capacity = t->rb_capacity + RB_SPACE;
    RBLANode * new_node_space = (RBLANode *)malloc(sizeof(RBLANode) * new_rb_capacity);
    if (!new_node_space){
        return -1;
    }
    memset(new_node_space, 0, sizeof(RBLANode) * new_rb_capacity);
    memmove(new_node_space, t->rb_node_space, sizeof(RBLANode) * t->rb_capacity);
    t->root = (RBNode*)(new_node_space + ((RBLANode*)t->root - t->rb_node_space));
    _relocal_node_t(t, t->root, new_node_space);
    t->nil->child[0] = t->nil->child[1] = t->nil->p = NULL;
    _relocal_node_free(t, new_node_space);
    t->p = new_node_space + (t->p - t->rb_node_space);
    free(t->rb_node_space);
    t->rb_node_space = new_node_space;
    t->rb_capacity = new_rb_capacity;
    return 0;
}

/* --------------------------------------
 * brief : create red black tree struct
 * para cmp_fn : compare function 
 * para free_fn: free function for pData
 * return : created red black tree 
 * -------------------------------------- */
RBTree * rb_create(CMP_FN cmp_fn, FREE_FN free_fn) {
    RBTree *rbt = (RBTree *) malloc(sizeof(RBTree));
    if (!rbt){
        goto rb_failed;
    }
    RBNode *nil = (RBNode *) malloc(sizeof(RBNode));
    if (!nil){
        goto nil_failed;
    }
    RBLANode * ns = (RBLANode*)malloc(sizeof(RBLANode) * RB_SPACE);
    if (!ns){
        goto ns_failed;
    }
    for (int i = 0; i < RB_SPACE - 1; i++){
        ns[i].next = ns + i + 1;
    }
    ns[RB_SPACE - 1].next = NULL;
    nil->color = RB_BLACK;
    nil->pData = nil->p = nil->child[0] = nil->child[1] = NULL;
    rbt->root = rbt->nil = nil;
    rbt->rb_node_space = ns;
    rbt->p = ns;
    rbt->rb_capacity = RB_SPACE;
    rbt->rb_count    = 0;
    rbt->cmp_fn = (CMP_FN)default_cmp;
    if (NULL != cmp_fn) {
        rbt->cmp_fn = cmp_fn;
    }
    rbt->free_fn = free_fn;
    return rbt;
ns_failed:
    free(nil);
nil_failed:
    free(rbt);
rb_failed:
    return NULL;
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
    if ((t->rb_count >= t->rb_capacity * 9 / 10) || (!t->p->next)){
        if (-1 == _extend_node_space(t)){
            return -1;
        }
    }
    RBNode *z;
    //z = (RBNode *) malloc(sizeof(RBNode));
    z = (RBNode*)t->p;
    t->p = t->p->next;
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
    t->rb_count += 1;
    return 0;
}

/* --------------------------------------
 * brief : delete pdata from rb tree
 * para t: rb tree pointer
 * para pdata: data pointer
 * return : success 0, failed -1
 * -------------------------------------- */
int rb_delete(RBTree *t, void *pdata) {
    if (t->rb_count == 0){
        return -1;
    }
    RBNode *z = rb_find(t, pdata);
    if (!z) return -1;
    RBNode *y = z, *x = NULL;
    RB_NODE_COLOR color = y->color;
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
    //free(z);
    //z = NULL;
    ((RBLANode*)z)->next = t->p;
    t->p = (RBLANode*)z;
    if (color == RB_BLACK) {
        rb_delete_fix(t, x);
    }
    t->rb_count -= 1;
    return 0;
}

/* --------------------------------------------
 * brief  : fetch space index of data in tree t
 * return : -1 if not found
 * -------------------------------------------- */
int  rb_index (RBTree * t, void *data){
    RBNode *z = rb_find(t, data);
    if (!z) return -1;
    return ((RBLANode*)z) - t->rb_node_space;
}

/* --------------------------------------
 * brief : clear the tree 
 *         Free the data space
 *         Not the node space!!
 *         you can still reuse the  tree
 * para t: rb tree
 * -------------------------------------- */
void rb_clear(RBTree * t){
    if (!t) return;
    if (t->free_fn){
        _del_node_t(t, t->root);
    }
    for (int i = 0; i < t->rb_capacity - 1; i++){
        t->rb_node_space[i].next = t->rb_node_space + i + 1;
    }
    t->rb_node_space[t->rb_capacity - 1].next = NULL;
    t->root = t->nil;
    t->p = t->rb_node_space;
    t->rb_count = 0;
}

/* -----------------------------------------------
 * brief : free the data space and tree node space
 *         destory the tree
 *         you can not use this tree again
 *         set tree to NULL after call rb_free!!! 
 * para t: rb tree
 * ----------------------------------------------- */
void rb_free(RBTree * t){
    if (!t) return ;
    rb_clear(t);
    free(t->rb_node_space);
    free(t->nil);
    t->root = t->nil = NULL;
    free(t);
}

void * rb_max_value(RBTree *t){
    RBNode * z = rb_mm(t, t->root, 1);
    if (z){
        return z->pData;
    }
    else{
        return NULL;
    }
}

void * rb_min_value(RBTree *t){
    RBNode * z = rb_mm(t, t->root, 0);
    if (z){
        return z->pData;
    }
    else{
        return NULL;
    }
}
