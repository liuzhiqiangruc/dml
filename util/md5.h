/* ========================================================
 *   Copyright (C) 2013 All rights reserved.
 *   
 *   filename : md5.h
 *   author   : liuzhiqiang01@baidu.com
 *   date     : 2013-11-26
 *   info     : gen the md5 digest for the input string
 * ======================================================== */

#ifndef _MD5_H
#define _MD5_H

typedef struct {
    unsigned int state[4];     
    unsigned int count[2];     
    unsigned char buffer[64];     
} MD5Context;
 
#define S11 7
#define S12 12
#define S13 17
#define S14 22
#define S21 5
#define S22 9
#define S23 14
#define S24 20
#define S31 4
#define S32 11
#define S33 16
#define S34 23
#define S41 6
#define S42 10
#define S43 15
#define S44 21
 
#define F(x, y, z) (((x) & (y)) | ((~x) & (z)))
#define G(x, y, z) (((x) & (z)) | ((y) & (~z)))
#define H(x, y, z) ((x) ^ (y) ^ (z))
#define I(x, y, z) ((y) ^ ((x) | (~z)))

#define ROTATE_LEFT(x, n) (((x) << (n)) | ((x) >> (32-(n))))

#define FF(a, b, c, d, x, s, ac)         do{             \
    (a) += F((b), (c), (d)) + (x) + (unsigned int)(ac);  \
    (a) = ROTATE_LEFT((a), (s));                         \
    (a) += (b);                                          \
}while(0);
 
#define GG(a, b, c, d, x, s, ac)         do{             \
    (a) += G((b), (c), (d)) + (x) + (unsigned int)(ac);  \
    (a) = ROTATE_LEFT((a), (s));                         \
    (a) += (b);                                          \
}while(0);
 
#define HH(a, b, c, d, x, s, ac)         do{             \
    (a) += H((b), (c), (d)) + (x) + (unsigned int)(ac);  \
    (a) = ROTATE_LEFT((a), (s));                         \
    (a) += (b);                                          \
}while(0);
 
#define II(a, b, c, d, x, s, ac)         do{             \
    (a) += I((b), (c), (d)) + (x) + (unsigned int)(ac);  \
    (a) = ROTATE_LEFT((a), (s));                         \
    (a) += (b);                                          \
}while (0);


/*
 * gen md5 digest for the input string "instr"
 * if instr is NULL or len(instr) == 0 return -1
 * else return 0 and the generated digest stored in the out_digest[16]
 * */
int gen_md5_digest(const char * instr, unsigned char out_digest[16]);

#endif //MD5_H
