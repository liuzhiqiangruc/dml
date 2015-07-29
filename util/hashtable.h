/* ========================================================
 *   Copyright (C) 2013 All rights reserved.
 *   
 *   filename : hashtable.h
 *   author   : liuzhiqiang01@baidu.com
 *   date     : 2013-11-17
 *   info     : key    : int|long|char*
 *              value  : char|short|int|long|float|double|char*
 *
 * ======================================================== */

#ifndef _HASHTABLE_H
#define _HASHTABLE_H

#define SUCCESS    0
#define FAILURE   -1
#define EXISTS     1
#define NOTEXISTS -2
#define VLEN       8
#define TNLEN     32

#define create_hashtable(size, ...)           \
       _create_hashtable(size, #__VA_ARGS__)

#define hash_add(ht,key,value)                \
       _hash_add((ht),(key),(value))

#define hash_find(ht,key,value)               \
       _hash_find((ht),(key),(value))

#define hash_del(ht,key)                      \
       _hash_del((ht),(key))

#define hash_exists(ht,key)                   \
       _hash_exists((ht),(key))

#define reset(ht)       ((ht)->pInternalPointer = (ht)->pListHead)
#define next(ht)        ((ht)->pInternalPointer = (ht)->pInternalPointer->pListNext)
#define isnotend(ht)    ((ht)->pInternalPointer != NULL)
#define nkey(ht)        ((ht)->pInternalPointer->h)
#define skey(ht)        ((ht)->pInternalPointer->key)
#define value(ht)       ((ht)->pInternalPointer->value)



typedef unsigned long ulong;
typedef unsigned int  uint;

typedef struct _bucket {
    ulong h;          /* hash value of key, keyvalue if key is a uint or ulong */
    char * key;       /* the point to key , if key is a string */
    char value[VLEN]; /* store a var of builtin type in a 8bit buffer */
    struct _bucket *pListNext;
    struct _bucket *pListLast;
    struct _bucket *pNext;
    struct _bucket *pLast;
} Bucket;

typedef struct _hashtable{
    int nTableSize;
    int nTableMask;
    int nNumOfElements;
    char keyType[TNLEN];
    char valueType[TNLEN];
    Bucket * pInternalPointer;
    Bucket * pListHead;
    Bucket * pListTail;
    Bucket ** arBuckets;
} HashTable;


HashTable * _create_hashtable(uint size, const char* s_typename);
int _hash_add(HashTable * ht, ...);
int _hash_find(HashTable * ht, ...);
int _hash_del(HashTable * ht, ...);
int _hash_exists(HashTable * ht, ...);
int  hash_num_elements(HashTable * ht);
void hash_free(HashTable * ht);



#endif //HASHTABLE_H
