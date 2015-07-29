/* ========================================================
 *   Copyright (C) 2015 All rights reserved.
 *   
 *   filename : hash_pattern.h
 *   author   : liuzhiqiang01@baidu.com
 *   date     : 2015-02-06
 *   info     : generate the dict pattern using lsh method
 * ======================================================== */

#ifndef _HASH_PATTERN_H
#define _HASH_PATTERN_H

/* --------------------------------------------------------------------
 * brief : generate patterns of time series
 * double * d : time series set
 * int n      : count of time series
 * int m      : length of each time series
 * int l      : length of pattern
 * int k      : count of hash function used
 * int *nlen  : count of  generated sub time series as pattern
 * return     : generated sub time series[rowid, startindex, clusterid]
 * --------------------------------------------------------------------- */
int (*get_hash_pattern(double * d, int n, int m, int l, int k, int *nlen))[3];

#endif //HASH_PATTERN_H

