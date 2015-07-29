/* ========================================================
 *   Copyright (C) 2013 All rights reserved.
 *   
 *   filename : hashtable.c
 *   author   : liuzhiqiang01@baidu.com
 *   date     : 2013-11-17
 *   info     : implementation of a simple hashtable
 * ======================================================== */

#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include "str.h"
#include "hashtable.h"

#define CONNECT_TO_BUCKET_DLLIST(element, list_head) do{    \
    (element)->pNext = (list_head);                         \
    (element)->pLast = NULL;                                \
    if ((element)->pNext) {                                 \
        (element)->pNext->pLast = (element);                \
    }                                                       \
}while(0);

#define DECONNECT_FROM_BUCKET_DLLIST(element,list_head) do{ \
    if((element)->pLast){                                   \
        (element)->pLast->pNext = (element)->pNext;         \
    }                                                       \
    else{                                                   \
        (list_head) = (element)->pNext;                     \
    }                                                       \
    if ((element)->pNext){                                  \
        (element)->pNext->pLast = (element)->pLast;         \
    }                                                       \
}while(0);

#define CONNECT_TO_GLOBAL_DLLIST(element, ht)    do{        \
    (element)->pListLast = (ht)->pListTail;                 \
    (ht)->pListTail = (element);                            \
    (element)->pListNext = NULL;                            \
    if ((element)->pListLast != NULL) {                     \
        (element)->pListLast->pListNext = (element);        \
    }                                                       \
    if (!(ht)->pListHead) {                                 \
        (ht)->pListHead = (element);                        \
    }                                                       \
    if ((ht)->pInternalPointer == NULL) {                   \
        (ht)->pInternalPointer = (element);                 \
    }                                                       \
}while(0);

#define DECONNECT_FROM_GLOBAL_DLLIST(element,ht)  do{            \
    if ((element)->pListNext){                                   \
        (element)->pListNext->pListLast = (element)->pListLast;  \
    }                                                            \
    else{                                                        \
        (ht)->pListTail = (element)->pListLast;                  \
    }                                                            \
    if ((element)->pListLast){                                   \
        (element)->pListLast->pListNext = (element)->pListNext;  \
    }                                                            \
    else{                                                        \
        (ht)->pListHead = (element)->pListNext;                  \
        (ht)->pInternalPointer = (element)->pListNext;           \
    }                                                            \
}while(0);


static ulong hash_func(char *arKey)
{
    register ulong hash = 5381;
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


HashTable * _create_hashtable(uint size, const char* s_typename){
    if (!s_typename || strlen(s_typename) == 0 || strlen(s_typename) >= TNLEN)
        return NULL;
    int  count;
    char types[TNLEN];
    strcpy(types, s_typename);
    char ** str_array = split(trim(types, 3), ',', &count);
    if (count != 2){
        free(str_array[0]);
        free(str_array);
        return NULL;
    }

    if (strcmp(trim(str_array[0],3),"int")  && 
        strcmp(trim(str_array[0],3),"long") && 
        strcmp(trim(str_array[0],3),"char*")){
        free(str_array[0]);
        free(str_array);
        return NULL;
    }

    if (strcmp(trim(str_array[1],3),"int"   )   &&
        strcmp(trim(str_array[1],3),"long"  )   &&
        strcmp(trim(str_array[1],3),"double")   &&
        strcmp(trim(str_array[1],3),"float" )   &&
        strcmp(trim(str_array[1],3),"short" )   &&
        strcmp(trim(str_array[1],3),"char*" )   &&
        strcmp(trim(str_array[1],3),"char")){
        free(str_array[0]);
        free(str_array);
        return NULL;
    }


    HashTable * ht = (HashTable*)malloc(sizeof(HashTable));
    if (!ht){
        free(str_array[0]);
        free(str_array);
        return NULL;
    }
    strcpy(ht->keyType,  trim(str_array[0],3));
    strcpy(ht->valueType,trim(str_array[1],3));
    free(str_array[0]);
    free(str_array);

    uint i = 3;
    if (size>= 0x80000000) {
        ht->nTableSize = 0x80000000;
    } else {
        while ((1U << i) < size) {
            i++;
        }
        ht->nTableSize = 1 << i;
    }
    ht->arBuckets = (Bucket **) malloc(ht->nTableSize * sizeof(Bucket *));
    if (!ht->arBuckets)
        return NULL;
    memset(ht->arBuckets,0,ht->nTableSize * sizeof(Bucket *));
    ht->nTableMask = ht->nTableSize - 1;                        
    ht->pListHead = NULL;
    ht->pListTail = NULL;
    ht->pInternalPointer = NULL;
    ht->nNumOfElements = 0;
    return ht;
}


int _hash_add(HashTable * ht, ...){
    ulong h;
    char * key = NULL;
    int keylen = 0;
    char value[8];
    uint nIndex;
    Bucket *p;

    va_list vlist;
    va_start(vlist,ht);
    if(strcmp(ht->keyType,"int") == 0){
        int k = va_arg(vlist,int);
        h = k;
    }
    else if(strcmp(ht->keyType,"long") == 0){
        long k = va_arg(vlist,long);
        h = k;
    }
    else if (strcmp(ht->keyType, "char*") == 0){
        char* k = va_arg(vlist,char*);
        h = hash_func(k);
        key = k;
        keylen = strlen(key);
    }
    else {
        return FAILURE;
    }
    if  (strcmp(ht->valueType, "char") == 0)
        (*value) = (char)va_arg(vlist,int);
    else if (strcmp(ht->valueType, "short") == 0)
        (*(short*)value) = (short)va_arg(vlist,int);
    else if(strcmp(ht->valueType, "int") == 0)
        (*(int*)value) = va_arg(vlist,int);
    else if (strcmp(ht->valueType, "long") == 0)
        (*(long*)value) = va_arg(vlist,long);
    else if (strcmp(ht->valueType, "float") == 0)
        (*(float*)value) = (float)va_arg(vlist,double);
    else if (strcmp(ht->valueType, "double") == 0)
        (*(double*)value) = va_arg(vlist,double);
    else if (strcmp(ht->valueType, "char*") == 0){
        char * tmp_str = va_arg(vlist,char*);
        char * new_str = (char*)malloc(strlen(tmp_str)+1);
        strcpy(new_str,tmp_str);
        new_str[strlen(tmp_str)] = '\0';
        (*(char**)value) = new_str;
    }
    else 
        return FAILURE;
    va_end(vlist);

    nIndex = h & ht->nTableMask;
    p = ht->arBuckets[nIndex];

    while (p!= NULL){
        if (p->h == h){
            if ((strcmp(ht->keyType,"char*") != 0)){
                if (strcmp(ht->valueType,"char*") == 0){
                    free(*(char**)p->value);
                }
                memmove(p->value, value, 8);
                return SUCCESS;
            }
            else if(strcmp(p->key,key) == 0){
                if (strcmp(ht->valueType,"char*") == 0){
                    free(*(char**)p->value);
                }
                memmove(p->value, value, 8);
                return SUCCESS;
            }
        }
        p = p->pNext;
    }
    p = (Bucket *) malloc(sizeof(Bucket));
    if (!p ){
        if (strcmp(ht->valueType, "char*") == 0){
            free(*(char**)value);
        }
        return FAILURE;
    }

    p->h = h;
    p->key = NULL;
    memmove(p->value,value,8);
    if (0 == strcmp(ht->keyType,"char*")){
        p->key = (char*)malloc(keylen +1);
        memmove(p->key,key,keylen);
        p->key[keylen] = '\0';
    }

    CONNECT_TO_BUCKET_DLLIST(p, ht->arBuckets[nIndex]);
    CONNECT_TO_GLOBAL_DLLIST(p, ht);
    ht->arBuckets[nIndex] = p;
    ht->nNumOfElements+=1;
    return SUCCESS;
}


int _hash_find(HashTable * ht, ...){
    ulong h;
    char * key = NULL;
    int keylen = 0;
    uint nIndex = 0;
    Bucket *p = NULL;

    va_list vlist;
    va_start(vlist,ht);
    if (strcmp(ht->keyType,"int") == 0){
        int k = va_arg(vlist,int);
        h = k;
    }
    else if(strcmp(ht->keyType, "long") == 0){
        long k = va_arg(vlist,long);
        h = k;
    }
    else if (strcmp(ht->keyType, "char*") == 0){
        char * k = va_arg(vlist,char*);
        h = hash_func(k);
        key = k;
        keylen = strlen(key);
    }
    else{
        return FAILURE;
    }

    nIndex = h & ht->nTableMask;
    p = ht->arBuckets[nIndex];
    while (NULL != p){
        if (p->h == h){
            if ((strcmp(ht->keyType, "char*") != 0)|| 
                (strcmp(ht->keyType, "char*") == 0 && strcmp(p->key,key) == 0)){
                if (strcmp(ht->valueType, "char") == 0){
                    char * value = va_arg(vlist,char*);
                    *value = (char)(*(p->value));
                }
                else if (strcmp(ht->valueType, "short") == 0){
                    short * value = va_arg(vlist,short*);
                    *value = *((short*)p->value);
                }
                else if (strcmp(ht->valueType, "int")   == 0){
                    int * value = va_arg(vlist,int*);
                    *value = *((int*)p->value);
                }
                else if (strcmp(ht->valueType, "long")  == 0){
                    long * value = va_arg(vlist,long*);
                    *value = *((long*)p->value);
                }
                else if (strcmp(ht->valueType, "float") == 0){
                    float * value = va_arg(vlist,float*);
                    *value = *((float*)p->value);
                }
                else if (strcmp(ht->valueType, "double")== 0){
                    double * value = va_arg(vlist,double*);
                    *value = *((double*)p->value);
                }
                else if (strcmp(ht->valueType, "char*") == 0){
                    char ** value = va_arg(vlist,char**);
                    *value = *((char**)p->value);
                }
                else {
                    va_end(vlist);
                    return FAILURE;
                }
                va_end(vlist);
                return SUCCESS;
            }
        }
        p = p->pNext;
    }
    va_end(vlist);
    return NOTEXISTS;
}


int _hash_del(HashTable * ht, ...){
    ulong h;
    char * key = NULL;
    int keylen = 0;
    uint nIndex = 0;
    Bucket *p = NULL;

    va_list vlist;
    va_start(vlist,ht);
    if (strcmp(ht->keyType,"int") == 0){
        int k = va_arg(vlist,int);
        h = k;
    }
    else if(strcmp(ht->keyType, "long") == 0){
        long k = va_arg(vlist,long);
        h = k;
    }
    else if (strcmp(ht->keyType, "char*") == 0){
        char * k = va_arg(vlist,char*);
        h = hash_func(k);
        key = k;
        keylen = strlen(key);
    }
    else{
        return FAILURE;
    }
    va_end(vlist);

    nIndex = h & ht->nTableMask;
    p = ht->arBuckets[nIndex];
    while (NULL != p){
        if (p->h == h){
            if ((strcmp(ht->keyType, "char*") != 0)|| 
                (strcmp(ht->keyType, "char*") == 0 && strcmp(p->key,key) == 0)){
                DECONNECT_FROM_BUCKET_DLLIST(p,ht->arBuckets[nIndex]);
                DECONNECT_FROM_GLOBAL_DLLIST(p,ht);
                if (p->key) {free(p->key);p->key = NULL;}
                if (strcmp(ht->valueType, "char*") == 0) free(*(char**)p->value);
                free(p); p = NULL;
                ht->nNumOfElements -= 1;
                return SUCCESS;
            }
        }
        p = p->pNext;
    }

    return SUCCESS;
}


int _hash_exists(HashTable * ht, ...){
    ulong h;
    char * key = NULL;
    int keylen = 0;
    uint nIndex = 0;
    Bucket *p = NULL;

    va_list vlist;
    va_start(vlist,ht);
    if (strcmp(ht->keyType,"int") == 0){
        int k = va_arg(vlist,int);
        h = k;
    }
    else if(strcmp(ht->keyType, "long") == 0){
        long k = va_arg(vlist,long);
        h = k;
    }
    else if (strcmp(ht->keyType, "char*") == 0){
        char * k = va_arg(vlist,char*);
        h = hash_func(k);
        key = k;
        keylen = strlen(key);
    }
    else{
        return FAILURE;
    }
    va_end(vlist);

    nIndex = h & ht->nTableMask;
    p = ht->arBuckets[nIndex];
    while (NULL != p){
        if (p->h == h){
            if ((strcmp(ht->keyType, "char*") != 0)|| 
                (strcmp(ht->keyType, "char*") == 0 && strcmp(p->key,key) == 0)){
                return EXISTS;
            }
        }
        p = p->pNext;
    }
    return NOTEXISTS;
}


int hash_num_elements(HashTable *ht)
{
    return ht->nNumOfElements;
}


void hash_free(HashTable * ht){
    Bucket *p, *q;
    p = ht->pListHead;
    while (p != NULL) {
        q = p;
        p = p->pListNext;
        if (strcmp(ht->keyType,"char*") == 0 &&
            q->key){
            free(q->key);
            q->key = NULL;
        }
        if (strcmp(ht->valueType,"char*") == 0){
            free(*(char**)q->value);
        }
        free(q);
        q = NULL;
    }

    if (ht->arBuckets) {
        free(ht->arBuckets);
        ht->arBuckets = NULL;
    }
    free(ht);
}

/*
int main(){
    HashTable * t = create_hashtable(20,char*,int);
    hash_add(t,"nihao",2);
    int v = 0;
    hash_find(t,"nihao",&v);
    printf("%d\n",v);
    hash_add(t,"nihao",4);
    hash_find(t,"nihao",&v);
    printf("%d\n",v);
    return 0;
}*/
