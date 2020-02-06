/* ========================================================
 *   Copyright (C) 2018 All rights reserved.
 *   
 *   filename : cokus.h
 *   author   : ***
 *   date     : 2018-12-10
 *   info     : 
 * ======================================================== */

#ifndef _COKUS_H
#define _COKUS_H

#include <stdlib.h>

typedef unsigned long uint32;

#define N              (624)                 // length of state vector
#define M              (397)                 // a period parameter
#define K              (0x9908B0DFU)         // a magic constant
#define hiBit(u)       ((u) & 0x80000000U)   // mask all but highest   bit of u
#define loBit(u)       ((u) & 0x00000001U)   // mask all but lowest    bit of u
#define loBits(u)      ((u) & 0x7FFFFFFFU)   // mask     the highest   bit of u
#define mixBits(u, v)  (hiBit(u)|loBits(v))  // move hi bit of u to hi bit of v

#ifdef RAND_MAX
#undef RAND_MAX
#endif
#define RAND_MAX (0xFFFFFFFFU)

typedef struct _RInfo{
    uint32   state[N+1];    
    uint32   *next;        
    int      left;
    uint32   seed;
}RInfo;

RInfo * create_rinfo(uint32 seed){
    RInfo * rinfo = (RInfo*)calloc(1, sizeof(RInfo));
    rinfo->seed   = seed;
    rinfo->left   = -1;
    return rinfo;
}

void seedMT(RInfo * rinfo){
    register uint32 x = (rinfo->seed | 1U) & 0xFFFFFFFFU, *s = rinfo->state;
    register int    j;
    for(rinfo->left=0, *s++=x, j=N; --j;
            *s++ = (x*=69069U) & 0xFFFFFFFFU);
}

uint32 reloadMT(RInfo * rinfo) {
    uint32 * state = rinfo->state;
    register uint32 *p0=state, *p2=state+2, *pM=state+M, s0, s1;
    register int    j;
    if(rinfo->left < -1)
        seedMT(rinfo);
    rinfo->left=N-1, rinfo->next=state+1;
    for(s0=state[0], s1=state[1], j=N-M+1; --j; s0=s1, s1=*p2++)
        *p0++ = *pM++ ^ (mixBits(s0, s1) >> 1) ^ (loBit(s1) ? K : 0U);
    for(pM=state, j=M; --j; s0=s1, s1=*p2++)
        *p0++ = *pM++ ^ (mixBits(s0, s1) >> 1) ^ (loBit(s1) ? K : 0U);
    s1=state[0], *p0 = *pM ^ (mixBits(s0, s1) >> 1) ^ (loBit(s1) ? K : 0U);
    s1 ^= (s1 >> 11);
    s1 ^= (s1 <<  7) & 0x9D2C5680U;
    s1 ^= (s1 << 15) & 0xEFC60000U;
    return(s1 ^ (s1 >> 18));
}

uint32 randomMT(RInfo * rinfo) {
    uint32 y;
    if(--(rinfo->left) < 0)
        return(reloadMT(rinfo));
    y  = *(rinfo->next)++;
    y ^= (y >> 11);
    y ^= (y <<  7) & 0x9D2C5680U;
    y ^= (y << 15) & 0xEFC60000U;
    y ^= (y >> 18);
    return(y);
}

#undef N
#undef M
#undef K

#endif //COKUS_H
