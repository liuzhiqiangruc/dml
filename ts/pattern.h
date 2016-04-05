/* ========================================================
 *   Copyright (C) 2016 All rights reserved.
 *   
 *   filename : pattern.h
 *   author   : ***
 *   date     : 2016-01-15
 *   info     : 
 * ======================================================== */

#ifndef _PATTERN_H
#define _PATTERN_H

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
int (*get_pattern(double * d, int n, int m, int l, int k, int *nlen))[3];




#endif //PATTERN_H
