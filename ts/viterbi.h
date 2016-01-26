/* ========================================================
 *   Copyright (C) 2014 All rights reserved.
 *   
 *   filename : viterbi.h
 *   author   : liuzhiqiang01@baidu.com
 *   date     : 2014-12-19
 *   info     : smoothing for discrete sequence
 * ======================================================== */

#ifndef _VITERBI_H
#define _VITERBI_H

/* *********************************************
 * @brief     : discrete [0|1] vector smoothing 
 * @case      : [0,0,1,0,0,1,1,1,0,1,1,0,0,0,0]
 *              [0,0,0,0,0,1,1,1,1,1,1,0,0,0,0]
 *
 * @param l   : input vector 
 * @param o   : output[smoothed] vector
 * @param n   : vector length
 * @param r_t : rate for stat <--> stat
 * @param r_o : rate for stat <--> observ
 *
 * ******************************************/
int viterbi(int * l, int * o, int n, int r_t, int r_o);


#endif //VITERBI_H
