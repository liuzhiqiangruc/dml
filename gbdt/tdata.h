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
    int bin;                        /* binary data or not        */
    int row;                        /* num of instances          */
    int col;                        /* feature count of data     */
    double * y;                     /* labels of data            */
    unsigned int * l;               /* row cnt of per feautre    */
    unsigned int * cl;              /* cumulative row cnt ..     */
    unsigned int * ids;             /* row ids of feature        */
    double * vals;                  /* row feature value         */
    char (*id_map)[FKL];            /* feature id name mapping   */
}DTD;

DTD *(*load_data(char * train_input, char * test_input, int binary))[2];

void free_data(DTD *ts);

#endif //TDATA_H
