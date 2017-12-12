/* ========================================================
 *   Copyright (C) 2017 All rights reserved.
 *   
 *   filename : data.h
 *   author   : liuzhiqiangruc@126.com
 *   date     : 2017-09-21
 *   info     : regression data interface for file format:
 *              label\tfeature\tvalue\tfeature\tvalue  or
 *              label\tfeature\tfeature\tfeature
 * ======================================================== */

#ifndef _DATA_H
#define _DATA_H

#include "hash.h"

#define FKL 32

typedef enum {ROW    = 1, COL}       DT_STYLE;
typedef enum {BINARY = 1, NOBINARY}  FEA_TYPE;
typedef enum {INITED = 1, NO_INITED} FEA_SET;

typedef struct {
    DT_STYLE dt_style;                /* store style: row or column based   */
    FEA_TYPE fea_type;                /* feature type : binary or not       */
    FEA_SET  fea_set;                 /* with or without feature set inited */
    unsigned int row;                 /* row count of dataset               */
    unsigned int col;                 /* col count of dataset               */
    unsigned int tkn;                 /* token count of dataset             */
    unsigned int * len;               /* length of each row or col          */
    unsigned int * clen;              /* cumulate length of each row or col */
    unsigned int * ids;               /* token ids of datasetl              */
    double * vals;                    /* token values of dataset            */
    double * y;                       /* label or target need to fit        */
    char (*id_map)[FKL];              /* idmap of features                  */
} DATA;

DATA * data_load(const char * input, DT_STYLE dt_style, FEA_TYPE fea_type, FEA_SET fea_set, Hash * hs);

void data_free(DATA * ds);

#endif //DATA_H
