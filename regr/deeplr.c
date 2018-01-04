/* ========================================================
 *   Copyright (C) 2017 All rights reserved.
 *   
 *   filename : deeplr.c
 *   author   : liuzhiqiangruc@126.com
 *   date     : 2017-12-11
 *   info     : just support l2 norm for now
 * ======================================================== */


#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "auc.h"
#include "deeplr.h"

static double loss(double * x, DATA * ds, double * hy, int k, int c){
    double loss = 0.0, yest = 0.0, add = 0.0;
    int i, j, offs, len, l;
    double *e = (double*)calloc(k, sizeof(double));
    for (i = 0; i < ds->row; i++){
        offs = ds->clen[i];
        len  = ds->len[i];
        memset(e, 0, sizeof(double) * k);
        if (ds->fea_type == BINARY) for (j = 0; j < len; j++) for (l = 0; l < k; l++){
            e[l] += x[ds->ids[offs + j] * k + l];
        }
        else if (ds->fea_type == NOBINARY) for (j = 0; j < len; j++) for (l = 0; l < k; l++){
            e[l] += x[ds->ids[offs + j] * k + l] * ds->vals[offs + j];
        }
        yest = 0.0;
        for (l = 0; l < k; l++){
            e[l] = 1.0 / (1 + exp(-e[l]));
            yest += e[l] * x[c * k + l];
        }
        if (hy) hy[i] = yest;
        add = yest > 30.0 ? yest : (yest < -30.0 ? 0.0 : log(1.0 + exp(yest)));
        add -= (ds->y[i] > 0.0 ? yest : 0.0);
        loss += add;
    }
    return loss;
}

// repo the train and test loss and auc status
// and return the train data loss
static double lr_repo(REGR *regr){
    double train_loss, test_loss = 0.0, train_auc, test_auc = 0.0;
    double *train_hy = (double*)calloc(regr->train_ds->row, sizeof(double));
    double *test_hy  = NULL;
    train_loss = loss(regr->x, regr->train_ds, train_hy, regr->reg_p.k, regr->feature_len);
    train_auc  = auc(regr->train_ds->row, train_hy, regr->train_ds->y);
    fprintf(stderr, "train_loss : %.8f, train_auc : %.8f", train_loss, train_auc); 
    if (regr->test_ds){
        test_hy = (double*)calloc(regr->test_ds->row, sizeof(double));
        test_loss = loss(regr->x, regr->test_ds, test_hy, regr->reg_p.k, regr->feature_len);
        test_auc  = auc(regr->test_ds->row, test_hy, regr->test_ds->y);
        fprintf(stderr, ";  test_loss : %.8f, test_auc : %.8f", test_loss, test_auc); 
    }
    fprintf(stderr, "\n");
    return train_loss;
}

static double random_f(){
    return (1.0 + rand()) / (1.0 + RAND_MAX);
}

static int deeplr_learn (REGR * regr){
    int n, i, j, offs, len, k, l, c;
    double loss = 0.0, new_loss = 0.0, hx = 0.0, yest = 0.0, tmp = 0.0;
    double *x = regr->x;
    double *e = NULL;
    DATA * ds = regr->train_ds;
    k = regr->reg_p.k;
    c = regr->feature_len;
    tmp = sqrt(k);
    for(i = 0; i < (c + 1) * k; i++){
        x[i] = (random_f() - 0.5) / tmp;
    }
    e = (double *)calloc(k, sizeof(double));
    loss = lr_repo(regr);
    for (n = 1; n <= regr->reg_p.n; n++){
        for (i = 0; i < ds->row; i++){
            offs = ds->clen[i];
            len  = ds->len[i];
            memset(e, 0, sizeof(double) * k);
            if (ds->fea_type == BINARY) for (j = 0; j < len; j++) for (l = 0; l < k; l++){
                e[l] += x[ds->ids[offs + j] * k + l];
            }
            else if (ds->fea_type == NOBINARY) for (j = 0; j < len; j++) for (l = 0; l < k; l++){
                e[l] += x[ds->ids[offs + j] * k + l] * ds->vals[offs + j];
            }
            yest = 0.0;
            for (l = 0; l < k; l++){
                e[l] = 1.0 / (1 + exp(-e[l]));
                yest += e[l] * x[c * k + l];
            }
            hx = yest < -30.0 ? 0.0 : (yest > 30.0 ? 1.0 : 1.0 / (1.0 + exp(-yest)));
            for (l = 0; l < k; l++){
                tmp = (ds->y[i] - hx) * x[c * k + l] * e[l] * (1.0 - e[l]);
                if (ds->fea_type == BINARY) for (j = 0; j < len; j++){
                    x[ds->ids[offs + j] * k + l] += regr->reg_p.alpha* (tmp - regr->reg_p.gamma * x[ds->ids[offs + j] * k + l]);
                }
                else if (ds->fea_type == NOBINARY) for (j = 0; j < len; j++){
                    x[ds->ids[offs + j] * k + l] += regr->reg_p.alpha* (tmp * ds->vals[offs + j] - regr->reg_p.gamma * x[ds->ids[offs + j] * k + l]);
                }
                x[c * k + l] += regr->reg_p.alpha * ((ds->y[i] - hx) * e[l] - regr->reg_p.gamma * x[c * k + l]);
            }
        }
        if (n % regr->reg_p.s == 0){
            save_model(regr, n);
        }
        new_loss = lr_repo(regr);
        if (loss - new_loss <= regr->reg_p.toler){
            fprintf(stderr, "conv done!!!\n");
            break;
        }
        loss = new_loss;
    }
    free(e);
    e = NULL;
    return 0;
}

REGR * create_deeplr_model(){
    REGR * lr = create_model(deeplr_learn);
    return lr;
}
