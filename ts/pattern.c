/* ========================================================
 *   Copyright (C) 2016 All rights reserved.
 *   
 *   filename : pattern.c
 *   author   : ***
 *   date     : 2016-01-15
 *   info     : 
 * ======================================================== */

#include <math.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "pattern.h"

typedef unsigned long long ULL;

typedef struct _simple_hash_map {
    int *head;
    int *next;
    ULL *key;
    int sz;
    int mask;
} SimpleHashMap;

typedef struct _node {
    int sub[2];
    struct _node *next;
} Node;

typedef struct _cls_list {
    Node *head;
    Node *tail;
    int size;
} ClsList;

typedef struct _cls_list_s {
    ClsList *cls_s;
    int max_size;
    int sz;
} ClsListS;

SimpleHashMap *h_create(int n, int mask) {
    SimpleHashMap *shm = (SimpleHashMap *) malloc(sizeof(SimpleHashMap));
    shm->head = (int *) malloc(sizeof(int) * n);
    memset(shm->head, -1, sizeof(int) * n);
    shm->next = (int *) malloc(sizeof(int) * n);
    memset(shm->next, -1, sizeof(int) * n);
    shm->key = (ULL *) malloc(sizeof(ULL) * n);
    memset(shm->key, 0, sizeof(ULL) * n);
    shm->sz = 0;
    shm->mask = mask;
    return shm;
}

void h_free(SimpleHashMap *shm) {
    if (shm) {
        if (shm->head) {
            free(shm->head);
            shm->head = NULL;
        }
        if (shm->next) {
            free(shm->next);
            shm->next = NULL;
        }
        if (shm->key) {
            free(shm->key);
            shm->key = NULL;
        }
        free(shm);
    }
}

int h_find(SimpleHashMap *shm, ULL val) {
    int x = val % shm->mask;
    for (int i = shm->head[x]; i != -1; i = shm->next[i]) {
        if (shm->key[i] == val) return i;
    }
    return -1;
}

int h_insert(SimpleHashMap *shm, ULL val) {
    int x = val % shm->mask;
    shm->key[shm->sz] = val;
    shm->next[shm->sz] = shm->head[x];
    shm->head[x] = shm->sz++;
    return shm->head[x];
}

void cls_insert(ClsList *t, int row, int offset) {
    Node *p = (Node *) malloc(sizeof(Node));
    p->sub[0] = row;
    p->sub[1] = offset;
    p->next = NULL;
    t->size += 1;
    if (NULL == t->tail) {
        t->head = t->tail = p;
    }
    else {
        t->tail->next = p;
        t->tail = p;
    }
}

ClsListS *cls_s_create(int n) {
    ClsListS *s = (ClsListS *) malloc(sizeof(ClsListS));
    s->max_size = n;
    s->sz = 0;
    s->cls_s = (ClsList *) malloc(sizeof(ClsList) * n);
    for (int i = 0; i < n; i++) {
        s->cls_s[i].head = s->cls_s[i].tail = NULL;
        s->cls_s[i].size = 0;
    }
    return s;
}

void cls_s_free(ClsListS *t) {
    if (!t) return;
    for (int i = 0; i < t->sz; i++){
        Node * p = t->cls_s[i].head;
        Node * q = p;
        while (p != NULL) {
            q = p->next;
            free(p);
            p = q;
        }
    }
    free(t->cls_s);
    t->cls_s = NULL;
    free(t);
}

ULL *gen_base(int l, int k) {
    ULL *ret = (ULL *) malloc(sizeof(ULL) * l);
    memset(ret, 0, sizeof(ULL) * l);
    int tmp = 0;
    srand(time(NULL));
    for (int i = 0; i < l; i++) {
        for (int j = 0; j < k; j++) {
            tmp = rand() & 1;
            if (tmp) {
                ret[i] |= (1LLU << j);
            }
        }
    }
    return ret;
}

void mean_sd(double *x, int n, double *mean, double *sd) {
    double s = 0.0, ss = 0.0;
    for (int i = 0; i < n; i++) {
        s += x[i];
        ss += x[i] * x[i];
    }
    *mean = s / n;
    *sd = sqrt((ss - (*mean) * s) / (n - 1));
}

void z_norm(double *x, int n, double *nx){
    double mean, sd;
    mean_sd(x, n, &mean, &sd);
    memset(nx, 0, sizeof(double) * n);
    for (int i = 0; i < n; i++){
        nx[i] = (x[i] - mean) / sd;
    }
}

ULL hash_sign(double *x, ULL *base, int l, int k) {
    ULL ret = 0LLU;
    for (int i = 0; i < k; i++) {
        double tmp = 0;
        for (int j = 0; j < l; j++) {
            if (base[j] & (1LLU << i)) {
                tmp += x[j];
            }
            else {
                tmp -= x[j];
            }
        }
        if (tmp > 0) ret |= (1LLU << (k - i - 1));
    }
    return ret;
}

ClsListS *gen_cluster(double *d, int n, int m, int l, int k) {
    ULL *base = gen_base(l, k);
    SimpleHashMap *shm = h_create(1000000, 999997);
    ClsListS *clusters = cls_s_create(1000000);
    double *sx = (double *)malloc(sizeof(double) * l);
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j <= m - l; ++j) {
            z_norm(d + i * m + j, l, sx);
            ULL w = hash_sign(sx, base, l, k);
            int pos = h_find(shm, w);
            if (pos != -1) {
                int *sub = clusters->cls_s[pos].tail->sub;
                if (sub[0] == i && j - sub[1] < l) continue;
            } else {
                pos = h_insert(shm, w);
                clusters->sz += 1;
            }
            cls_insert(&clusters->cls_s[pos], i, j);
        }
    }
    free(base);     base = NULL;
    free(sx);       sx   = NULL;
    h_free(shm);    shm  = NULL;
    return clusters;
}

int (*filter_cluster(ClsListS *cls, int l, int *nlen))[3] {
    ClsListS *filter = cls_s_create(1000000);
    ClsListS *res = cls_s_create(1000000);
    for (int i = 0; i < cls->sz; ++i) {
        if (cls->cls_s[i].size < 2) continue;
        ClsList *cl = &res->cls_s[res->sz];
        res->sz += 1;
        Node *p = cls->cls_s[i].head;
        while (p != NULL) {
            Node *q = filter->cls_s[p->sub[0]].head;
            if (q == NULL) {
                cls_insert(cl, p->sub[0], p->sub[1]);
                p = p->next;
                continue;
            }
            int isOver = 0;
            while (q != NULL) {
                if (abs(q->sub[1] - p->sub[1]) < l) {
                    isOver = 1;
                    break;
                }
                q = q->next;
            }
            if (!isOver) cls_insert(cl, p->sub[0], p->sub[1]);
            p = p->next;
        }
        if (cl->size >= 2) {
            (*nlen) += cl->size;
            Node *q = cl->head;
            while (q != NULL) {
                cls_insert(&filter->cls_s[q->sub[0]], q->sub[0], q->sub[1]);
                q = q->next;
            }
        }
    }
    int (*ret)[3] = malloc(sizeof(int[3]) * (*nlen));
    int totRetClass = 0;
    for (int i = 0, pos = 0; i < res->sz; ++i) {
        if (res->cls_s[i].size < 2) continue;
        Node *p = res->cls_s[i].head;
        while (p != NULL) {
            ret[pos][0] = totRetClass;
            ret[pos][1] = p->sub[0];
            ret[pos][2] = p->sub[1];
            pos += 1;
            p = p->next;
        }
        totRetClass++;
    }
    cls_s_free(cls);
    cls_s_free(filter);
    cls_s_free(res);
    return ret;
}

int (*get_pattern(double *d, int n, int m, int l, int k, int *nlen))[3] {
    ClsListS *cluster = gen_cluster(d, n, m, l, k);
    return filter_cluster(cluster, l, nlen);
}
