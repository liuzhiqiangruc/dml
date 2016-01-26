/* ========================================================
 *   Copyright (C) 2015 All rights reserved.
 *   
 *   filename : hash.c
 *   author   : liuzhiqiangruc@126.com
 *   date     : 2015-12-26
 *   info     : Hash index for uint ulong & string
 * ======================================================== */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include "hash.h"

#define HASH_SPACE 0x100000

// Hash Val
typedef union _hash_val {
    unsigned int uival;
    unsigned long long ulval;
    char * strval;
} HashVal;

// linked array node for HashVal
typedef union _la_node {
    HashVal value;
    union _la_node * next;
} LANode;

// hash struct 
struct _hash {
    int size;           // number of buckets 2^n
    int count;          // number of elements inserted
    int mask;           // size - 1
    int space;          // capacity of hash index
    int * head;         // bucket head pointer
    int * next;         // internal pointer in bucket
    LANode * vals;      // data space for HashVal
    LANode * p;         // current aviliable node
    HashType type;      // INT, LONG, STRING
};

// hash function for string 
static unsigned long long hash_func(char *arKey)
{
    register unsigned long long hash = 5381;
    int      nKeyLength = strlen(arKey);
    for (; nKeyLength >= 8; nKeyLength -= 8) {
        hash = ((hash << 5) + hash) + *arKey++;
        hash = ((hash << 5) + hash) + *arKey++;
        hash = ((hash << 5) + hash) + *arKey++;
        hash = ((hash << 5) + hash) + *arKey++;
        hash = ((hash << 5) + hash) + *arKey++;
        hash = ((hash << 5) + hash) + *arKey++;
        hash = ((hash << 5) + hash) + *arKey++;
        hash = ((hash << 5) + hash) + *arKey++;
    }
    switch (nKeyLength) {
        case 7: hash = ((hash << 5) + hash) + *arKey++; /* fallthrough... */
        case 6: hash = ((hash << 5) + hash) + *arKey++; /* fallthrough... */
        case 5: hash = ((hash << 5) + hash) + *arKey++; /* fallthrough... */
        case 4: hash = ((hash << 5) + hash) + *arKey++; /* fallthrough... */
        case 3: hash = ((hash << 5) + hash) + *arKey++; /* fallthrough... */
        case 2: hash = ((hash << 5) + hash) + *arKey++; /* fallthrough... */
        case 1: hash = ((hash << 5) + hash) + *arKey++; break;
        case 0: break;
        default:
                break;
    }
    return hash;
}

// return hash code for HashVal v
static unsigned long long hash_code(Hash * hs, HashVal v){
    unsigned long long hash_code = 0UL;
    switch (hs->type) {
        case INT:
            hash_code = v.uival;
            break;
        case LONG:
            hash_code = v.ulval;
            break;
        case STRING:
            hash_code = hash_func(v.strval);
            break;
        default :
            break;
    }
    return hash_code;
}

//deep copy for HashVal
static HashVal copy_val(Hash * hs, HashVal s){
    HashVal t = s;
    if (hs->type == STRING){
        int slen = strlen(s.strval);
        char * news = (char*)malloc(sizeof(char) * (slen + 1));
        memmove(news, s.strval, slen);
        news[slen] = '\0';
        t.strval = news;
    }
    return t;
}

// extend the storage space for vals 
static void extend_hash(Hash * hs){
    LANode * oldvals = hs->vals;
    LANode * q = hs->p;
    LANode * p = NULL;
    hs->vals = (LANode *)malloc(sizeof(LANode) * (hs->space + HASH_SPACE));
    memmove(hs->vals, oldvals, sizeof(LANode) * hs->space);
    hs->p = hs->vals + (q - oldvals);
    p = hs->p;
    while (q && q->next){
        q = q->next;
        p->next = hs->vals + (q - oldvals);
        p = p->next;
    }
    free(oldvals);
    oldvals = NULL;
    for (int i = 0; i < HASH_SPACE; i++){
        p->next = hs->vals + hs->space + i;
        p = p->next;
    }
    p->next = NULL;
    hs->space += HASH_SPACE;
    hs->next = realloc(hs->next, sizeof(int) * hs->space);
}

// resize the hash buckets by twice
static void resize_hash(Hash * hs){
    int nsize = hs->size << 1;
    int nmask = nsize - 1;
    int * nhead = (int*)malloc(sizeof(int) * nsize);
    int * nnext = (int*)malloc(sizeof(int) * hs->space);
    memset(nhead, -1, sizeof(int) * nsize);
    memset(nnext, -1, sizeof(int) * hs->space);
    for (int i = 0; i < hs->size; i++){
        for (int j = hs->head[i]; j != -1; j = hs->next[j]){
            HashVal v = hs->vals[j].value;
            unsigned long long h_code = hash_code(hs, v);
            int x = h_code & nmask;
            nnext[j] = nhead[x];
            nhead[x] = j;
        }
    }
    free(hs->head);
    free(hs->next);
    hs->head = nhead;
    hs->next = nnext;
    hs->size = nsize;
    hs->mask = nmask;
}

// create and init the hash struct
Hash * hash_create(int n, HashType type){
    Hash * hs = (Hash*)malloc(sizeof(Hash));
    if (!hs){
        goto hs_failed;
    }
    memset(hs, 0, sizeof(Hash));
    if (n > HASH_SPACE){
        hs->size = HASH_SPACE;
    }
    else {
        int i = 2;
        while ((1U << i) < n){
            i += 1;
        }
        hs->size = (1U << i);
    }
    hs->count = 0;
    hs->mask = hs->size - 1;
    hs->space = HASH_SPACE;
    hs->head = (int *)malloc(sizeof(int) * hs->size);
    if(!hs->head){
        goto head_failed;
    }
    memset(hs->head, -1, sizeof(int) * hs->size);
    hs->next = (int *)malloc(sizeof(int) * hs->space);
    if (!hs->next){
        goto next_failed;
    }
    memset(hs->next, -1, sizeof(int) * hs->space);
    hs->vals = (LANode*)malloc(sizeof(LANode) * hs->space);
    if (!hs->vals){
        goto vals_failed;
    }
    for (int i = 0; i < hs->space - 1; i++){
        hs->vals[i].next = hs->vals + i + 1;
    }
    hs->vals[hs->space - 1].next = NULL;
    hs->p = hs->vals;
    hs->type = type;
    return hs;
vals_failed:
    free(hs->next);
next_failed:
    free(hs->head);
head_failed:
    free(hs);
hs_failed:
    return NULL;
}

// find HashVal v in Hash struct hs
// and return the corelated hash index
// return -1 if not found
int hash_find(Hash * hs, ...){
    HashVal v;
    va_list vlist;
    va_start(vlist, hs);
    switch (hs->type){
        case INT:
            v.uival = va_arg(vlist, unsigned int);
            break;
        case LONG:
            v.ulval = va_arg(vlist, unsigned long long);
            break;
        case STRING:
            v.strval = va_arg(vlist, char*);
            break;
        default:
            break;
    }
    va_end(vlist);
    unsigned long long h_code = hash_code(hs, v);
    int x = h_code & hs->mask;
    for (int i = hs->head[x]; i != -1; i = hs->next[i]){
        if (hs->type == INT && hs->vals[i].value.uival == v.uival){
            return i;
        }
        else if (hs->type == LONG && hs->vals[i].value.ulval == v.ulval){
            return i;
        }
        else if (hs->type == STRING && (strcmp(hs->vals[i].value.strval, v.strval) == 0)){
            return i;
        }
    }
    return -1;
}

// insert a HashVal v into a Hash index struct
int hash_add(Hash * hs, ...) { 
    int i  = -1;
    HashVal v;
    va_list vlist;
    va_start(vlist, hs);
    switch (hs->type){
        case INT:
            v.uival = va_arg(vlist, unsigned int);
            i = hash_find(hs, v.uival);
            break;
        case LONG:
            v.ulval = va_arg(vlist, unsigned long long);
            i = hash_find(hs, v.ulval);
            break;
        case STRING:
            v.strval = va_arg(vlist, char*);
            i = hash_find(hs, v.strval);
            break;
        default:
            break;
    }
    va_end(vlist);
    if (i == -1){
        if ((hs->count > hs->space * 9 / 10) || (!(hs->p)) || (!(hs->p->next))){
            extend_hash(hs);
        }
        if (hs->count == hs->size){
            resize_hash(hs);
        }
        unsigned long long h_code = hash_code(hs, v);
        int x = h_code & hs->mask;
        LANode * q = hs->p;
        hs->p = hs->p->next;
        q->value = copy_val(hs, v);
        i = q - hs->vals;
        hs->next[i] = hs->head[x];
        hs->head[x] = i;
        hs->count += 1;
    }
    return i;
}

// del HashVal v from hs
// return the old index of v
int hash_del(Hash * hs, ...){
    HashVal v;
    va_list vlist;
    va_start(vlist, hs);
    switch (hs->type){
        case INT:
            v.uival = va_arg(vlist, unsigned int);
            break;
        case LONG:
            v.ulval = va_arg(vlist, unsigned long long);
            break;
        case STRING:
            v.strval = va_arg(vlist, char*);
            break;
        default:
            break;
    }
    va_end(vlist);
    unsigned long long h_code = hash_code(hs, v);
    int x = h_code & hs->mask;
    int i, j = -1;
    for (i = hs->head[x]; i != -1; i = hs->next[i]){
        if (hs->type == INT && hs->vals[i].value.uival == v.uival){
            break;
        }
        else if (hs->type == LONG && hs->vals[i].value.ulval == v.ulval){
            break;
        }
        else if (hs->type == STRING && (strcmp(hs->vals[i].value.strval, v.strval) == 0)){
            free(hs->vals[i].value.strval);
            break;
        }
        j = i;
    }
    if (i != -1){
        hs->vals[i].next = hs->p;
        hs->p = hs->vals + i;
        if (j == -1){
            hs->head[x] = hs->next[i];
            hs->next[i] = -1;
        }
        else{
            hs->next[j] = hs->next[i];
        }
        hs->count -= 1;
    }
    return i;
}

int hash_cnt(Hash * hs){
    return hs->count;
}

// free the hash index struct
void hash_free(Hash * hs){
    if (hs){
        if (hs->vals){
            if (hs->type == STRING){
                for (int i = 0; i < hs->size; i++){
                    for (int j = hs->head[i]; j != -1; j = hs->next[j]){
                        free(hs->vals[j].value.strval);
                    }
                }
            }
            free(hs->vals);
        }
        if (hs->head){
            free(hs->head);
        }
        if (hs->next){
            free(hs->next);
        }
        free(hs);
    }
}

/*
int main(){
    Hash * hs = hash_create(600, INT);
    int i = 0; 
    int index = -1;
    while (i < 600){
        index = hash_add(hs, i);
        printf("value: %d\thash_index:%d\n", i, index);
        i += 4;
    }
    printf("del\n");
    i = 0;
    while (i < 100){
        index = hash_del(hs, i);
        if (index != -1)
            printf("value : %d \t hash_index : %d\n", i, index);
        i += 1;
    }
    printf("insert after del\n");
    i = 100;
    while (i < 600){
        index = hash_add(hs, i);
        printf("value : %d \t hash_index : %d\n", i, index);
        i += 1;
    }

    i = 0;
    printf("found\n");
    while (i < 600){
        index = hash_find(hs, i);
        if (index != -1)
            printf("value : %d \t hash_index : %d\n", i, index);
        i += 1;
    }
    hash_free(hs);
    return 0;
}
int main(){
    char * name = "liuzhiqiang";
    char * name1 = "liuzhiqiang01";
    char * name2 = "liuzhiqiang";
    Hash * hs = hash_create(100, STRING);
    printf("%d\n", hash_add(hs, name));
    printf("%d\n", hash_add(hs, name1));
    printf("%d\n", hash_add(hs, name2));
    printf("%d\n", hash_add(hs, "liuzhiqiang01"));
    printf("%d\n", hash_add(hs, "liuzhiqiang02"));
    printf("%d\n", hash_add(hs, "liuzhiqiang02"));
    printf("%d\n", hash_add(hs, "liuzhiqiang02"));
    printf("%d\n", hash_add(hs, "liuzhiqiang02"));
    printf("%d\n", hash_add(hs, "liuzhiqiang01"));
    printf("%d\n", hash_add(hs, "liuzhiqiang01"));
    printf("%d\n", hash_add(hs, "liuzhiqiang03"));
    printf("%d\n", hash_add(hs, "liuzhiqiang04"));
    printf("%d\n", hash_del(hs, "liuzhiqiang"));
    printf("%d\n", hash_add(hs, "liuzhiqiang05"));
    printf("%d\n", hash_add(hs, "liuzhiqiang06"));
    hash_free(hs);
    return 0;
}
*/
