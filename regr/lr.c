/* ========================================================
 *   Copyright (C) 2017 All rights reserved.
 *   
 *   filename : lr.c
 *   author   : ***
 *   date     : 2017-12-11
 *   info     : 
 * ======================================================== */

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "auc.h"
#include "lr.h"

static double l1_norm(double x, double g, double lambda){
    if (x > 0.0){
        g += lambda;
    }
    else if (x < 0.0){
        g -= lambda;
    }
    else if (g > lambda){
        g -= lambda;
    }
    else if (g < -lambda){
        g+= lambda;
    }
    return g;
}

static double loss(double * x, DATA * ds, double * hy){
    double loss = 0.0, yest = 0.0, add = 0.0;
    int i, j;
    for (i = 0; i < ds->row; i++){
        yest = 0.0;
        for (j = 0; j < ds->len[i]; j++){
            yest += x[ds->ids[ds->clen[i] + j]] * (ds->fea_type == BINARY ? 1.0 : ds->vals[ds->clen[i] + j]);
        }
        if (hy) hy[i] = yest;
        add = yest > 30.0 ? yest : (yest < -30.0 ? 0.0 : log(1.0 + exp(yest)));
        add -= (ds->y[i] > 0 ? yest : 0.0);
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
    train_loss = loss(regr->x, regr->train_ds, train_hy);
    train_auc  = auc(regr->train_ds->row, train_hy, regr->train_ds->y);
    fprintf(stderr, "train_loss : %.8f, train_auc : %.8f", train_loss, train_auc); 
    if (regr->test_ds){
        test_hy = (double*)calloc(regr->test_ds->row, sizeof(double));
        test_loss = loss(regr->x, regr->test_ds, test_hy);
        test_auc  = auc(regr->test_ds->row, test_hy, regr->test_ds->y);
        fprintf(stderr, ";  test_loss : %.8f, test_auc : %.8f", test_loss, test_auc); 
    }
    fprintf(stderr, "\n");
    return train_loss;
}

#define sign(x) ((x)>0.0?1:((x)<0.0?-1:0))

static int lr_learn (REGR * regr){
    int n, i, j, offs, len;
    double *g = NULL;
    double delta = 0.0, loss = 0.0, new_loss = 0.0, hx = 0.0, yest = 0.0;
    DATA * ds = regr->train_ds;
    g = (double*)calloc(regr->feature_len, sizeof(double));
    loss = lr_repo(regr);
    for (n = 1; n <= regr->reg_p.n; n++){
        for (i = 0; i < ds->row; i++){
            offs = ds->clen[i];
            len  = ds->len[i];
            yest = 0.0;
            if (ds->fea_type == BINARY) for (j = 0; j < len; j++){
                yest += regr->x[ds->ids[offs + j]];
            }
            else if (ds->fea_type == NOBINARY) for (j = 0; j < len; j++){
                yest += regr->x[ds->ids[offs + j]] * ds->vals[offs + j];
            }
            hx = yest < -30.0 ? 0.0 : (yest > 30.0 ? 1.0 : 1.0 / (1.0 + exp(-yest)));
            if (ds->fea_type == BINARY) for (j = 0; j < len; j++) {
                g[ds->ids[offs + j]] = hx - ds->y[i];
            }
            else if (ds->fea_type == NOBINARY) for (j = 0; j < len; j++){
                g[ds->ids[offs + j]] = (hx - ds->y[i]) * ds->vals[offs + j];
            }
            for (j = 0; j < len; j++){
                if (regr->reg_p.r == 2) g[ds->ids[offs + j]] += regr->reg_p.gamma * regr->x[ds->ids[offs + j]];
                if (regr->reg_p.r == 1) g[ds->ids[offs + j]]  = l1_norm(regr->x[ds->ids[offs + j]], g[ds->ids[offs + j]], regr->reg_p.gamma);
            }
            for (j = 0; j < len; j++){
                delta = regr->reg_p.alpha * g[ds->ids[offs + j]];
                if (regr->reg_p.r == 1) if (sign(regr->x[ds->ids[offs + j]]) * (regr->x[ds->ids[offs + j]] - delta) < 0.0){
                    delta = regr->x[ds->ids[offs + j]];
                }
                regr->x[ds->ids[offs + j]] -= delta;
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
    free(g);
    g = NULL;
    return 0;
}

REGR * create_lr_model(){
    REGR * lr = create_model(lr_learn);
    return lr;
}
