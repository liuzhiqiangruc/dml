/* ========================================================
 *   Copyright (C) 2014 All rights reserved.
 *   
 *   filename : fptree.h
 *   author   : liuzhiqiang01@baidu.com
 *   date     : 2014-07-09
 *   info     : frequent pattern tree
 * ======================================================== */

#ifndef _FPTREE_H
#define _FPTREE_H

#define KEY_SIZE 64               /* max key length */
#define INIT_NODE_SIZE 10000000   /* just alloc 240M for fpt init */


/* -----------------------------------------------------
 * Data Define
 * ----------------------------------------------------- */

typedef struct __fpnode{          /* fpt node struct */
    int item;                     /* item id */
    int supp;                     /* item sup */
    int ci;                       /* child index in node space */
    int si;                       /* sibling index in node space */
    int li;                       /* link index in node space */
    int pi;                       /* parent index in node sapce */
} FPTNODE;


typedef struct __fplist{          /* element of header table */
    int supp;                     /* freq 1 item set sup */
    int itemlink;                 /* link index of first node of these with same item id */
    char itemname[KEY_SIZE];      /* item name */
} FPTLIST;


typedef struct __fptree{          /* fpt struct */
    int minsup;                   /* min sup for the tree */
    int headsize;                 /* length of header table */
    unsigned int nodespace;       /* totle space size for node */
    unsigned int usindex;         /* current using fpt node in space */
    FPTLIST * header;             /* header table */
    FPTNODE nodelist[1];          /* pre init fpt node space */
} FPTREE;




/* ----------------------------------------------------- 
 * Functions
 * ----------------------------------------------------- */

FPTREE * create_fpt(int minsup);
FPTREE * build_fpt(const char * infile, FPTREE * fpt, char sep);

int      select_ids_by_key(const char * key, FPTREE * fpt, int ids[], int count, int sup_threadhold);
void     get_sup_by_key(int n, FPTREE * fpt, int ids[], int count, int out_sup[]);

void     print_fpt(FPTREE * fpt);
void     free_fpt(FPTREE * fpt);

void     save_fpt(FPTREE * fpt, const char * serfile);
FPTREE * load_fpt(const char * serfile);

#endif //FPTREE_H
