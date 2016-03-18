/* ========================================================
 *   Copyright (C) 2016 All rights reserved.
 *   
 *   filename : tdata.h
 *   author   : liuzhiqiangruc@126.com
 *   date     : 2016-03-05
 *   info     : 
 * ======================================================== */

#ifndef _TDATA_H
#define _TDATA_H

#define FKL 32

typedef struct {
    int id;
    double val;
} DPair;

typedef struct {
    int row;                        /* num of instances          */
    double * y;                     /* labels of data            */
    int pos;                        /* +1 labels  cnt            */
    int neg;                        /* -1|0 lables  cnt          */
    int col;                        /* feature count of data     */
    int * l;                        /* row cnt of per feautre    */
    int * cl;                       /* cumulative row cnt ..     */
    int * ids;                      /* row ids of feature        */
    double * vals;                  /* row feature value         */
    char (*id_map)[FKL];            /* feature id name mapping   */
}DTD;

DTD *(*load_data(char * train_input, char * test_input, int binary))[2];

void free_data(DTD *ts);

#endif //TDATA_H
