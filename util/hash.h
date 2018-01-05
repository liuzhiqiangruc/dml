/* ========================================================
 *   Copyright (C) 2015 All rights reserved.
 *   
 *   filename : hash.h
 *   author   : liuzhiqiangruc@126.com
 *   date     : 2015-12-26
 *   info     : generating unique continous increasing id
 *              for any noncontinous uint, ulong or string
 * ======================================================== */

#ifndef _HASH_H
#define _HASH_H

// Hash Types
typedef enum { INT, LONG, STRING } HashType;

// Hash struct
typedef struct _hash Hash;

// create hash index struct 
// n    :  bucket size
// type :  INT, LONG or STRING
Hash * hash_create(int n, HashType type);

// find hash index of v in hash index struct
int  hash_find(Hash * hs, ...);

// add a var v into hash index struct 
// and return the hash index 
int  hash_add (Hash * hs, ...);

// del a val v from hash index struct
// and return the old index of v
int  hash_del (Hash * hs, ...);

// clean the hash for new using
void hash_clean(Hash * hs);

// number of elements
int  hash_cnt (Hash * hs);

// size of hash
int  hash_size(Hash * hs);

// free hash index struct
void hash_free(Hash * hs);

#endif //HASH_H

