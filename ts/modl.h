/* ========================================================
 *   Copyright (C) 2014 All rights reserved.
 *   
 *   filename : modl.h
 *   author   : liuzhiqiang01@baidu.com
 *   date     : 2014-10-17
 *   info     : implemention for the modl algorithm
 *              Based on the paper:
 *              Modl : A Bayes optimal discretization method 
 *                     for contunuous attributes
 * ======================================================== */

#ifndef _MODL_H
#define _MODL_H

/* ----------------------------------------------
 * brief    : init the LogD for modl
 * int L    : the length of LogD
 * return   : the LogD dict
 * ---------------------------------------------- */
double * initLogD(int L);

/* ----------------------------------------------
 * brief : the interface for the modl algorithm
 * double v : the value array
 * int a    : the label array
 * double LogD: the Log Dict Array
 * int n    : the length of array
 * int *nd  : the length of split rule point 
 * return   : the split rule array
 *            need to free after using
 * ---------------------------------------------- */
double * modl(double v[], int a[], double * LogD, int n, int* nd);

#endif //MODL_H

