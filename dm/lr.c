/* ========================================================
 *   Copyright (C) 2015 All rights reserved.
 *   
 *   filename : lr.c
 *   author   : liuzhiqiang01@baidu.com
 *   date     : 2015-08-14
 *   info     : implementation for LR with L1 L2 norm
 *              using newton method
 *              support binary or realvalued features
 * ======================================================== */

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "str.h"
#include "idmap.h"
#include "fn_type.h"
#include "newton_opt.h"
#include "auc.h"
#include "lr.h"

#define  LR_LINE_LEN  1048576 
#define  LR_KEY_LEN   64

static int load_train_ds(LR * lr, IdMap * idmap){
    FILE * fp = NULL;
    if (NULL == (fp = fopen(lr->p.in_file, "r"))){
        fprintf(stderr, "can not open file \"%s\"\n", lr->p.in_file);
        return -1;
    }
    lr->train_ds = (LRDS*)malloc(sizeof(LRDS));
    memset(lr->train_ds, 0, sizeof(LRDS));
    char buffer[LR_LINE_LEN];
    char ** str_array = NULL;
    int count = 0, col_cnt = 0, tf_cnt = 0, row = 0;

    // first scan for counting and idmapping
    while (NULL != fgets(buffer, LR_LINE_LEN, fp)) {
        str_array = split(trim(buffer, 3), '\t', &count);
        if (count < 3){
            goto free_str;
        }
        int tlen = atoi(str_array[1]);
        if (1 == lr->p.binary && count != (tlen + 2)){
            goto free_str;
        }
        if (0 == lr->p.binary && count != ((tlen + 1) << 1)){
            goto free_str;
        }
        for (int i = 0; i < tlen; i++){
            int id = 0;
            if (1 == lr->p.binary) id = i + 2;
            else id = (i << 1) + 2;
            if (-1 == idmap_get_value(idmap, str_array[id])) {
                idmap_add(idmap, dupstr(str_array[id]), idmap_size(idmap));
            }
        }
        tf_cnt += tlen;
        row += 1;
free_str:
        free(str_array[0]);
        free(str_array);
    }
    col_cnt = idmap_size(idmap);

    lr->id_map = (char(*)[LR_KEY_LEN])malloc(sizeof(char[LR_KEY_LEN]) * col_cnt);
    memset(lr->id_map, 0, sizeof(char[LR_KEY_LEN]) * col_cnt);
    lr->c = col_cnt;
    lr->x = (double*)malloc(sizeof(double) * col_cnt);
    memset(lr->x, 0, sizeof(double) * col_cnt);

    lr->train_ds->r   = row;
    lr->train_ds->y   = (double*)malloc(sizeof(double) * row);
    lr->train_ds->l   = (int*)malloc(sizeof(int) * row);
    lr->train_ds->ids = (int*)malloc(sizeof(int) * tf_cnt);
    if (0 == lr->p.binary){
        lr->train_ds->val = (double*)malloc(sizeof(double) * tf_cnt);
    }

    // second scan for loading the data to malloced space
    rewind(fp);
    tf_cnt = row = 0;
    while (NULL != fgets(buffer, LR_LINE_LEN, fp)){
        str_array = split(trim(buffer, 3), '\t', &count);
        if (count < 3) {
            goto str_free;
        }
        int tlen = atoi(str_array[1]);
        if (1 == lr->p.binary && count != (tlen + 2)){
            goto str_free;
        }
        if (0 == lr->p.binary && count != ((tlen + 1) << 1)) {
            goto str_free;
        }
        lr->train_ds->l[row] = tlen;
        lr->train_ds->y[row] = atof(str_array[0]);
        for (int i = 0; i < tlen; i++){
            int id = 0;
            if (1 == lr->p.binary) id = i + 2;
            else id = (i << 1) + 2;
            int iid = idmap_get_value(idmap, str_array[id]);
            lr->train_ds->ids[tf_cnt] = iid;
            strncpy(lr->id_map[iid], str_array[id], LR_KEY_LEN - 1);
            if (0 == lr->p.binary){
                lr->train_ds->val[tf_cnt] = atof(str_array[id + 1]);
            }
            tf_cnt += 1;
        }
        row += 1;
str_free:
        free(str_array[0]);
        free(str_array);
    }
    fclose(fp);
    return 0;
}

static void load_test_ds(LR * lr, IdMap * idmap){
    FILE * fp = NULL;
    if (NULL == (fp = fopen(lr->p.te_file, "r"))){
        return;
    }
    lr->test_ds = (LRDS*)malloc(sizeof(LRDS));
    memset(lr->test_ds, 0, sizeof(LRDS));
    char buffer[LR_LINE_LEN];
    char ** str_array = NULL;
    int count = 0, col_cnt = 0, tf_cnt = 0, row = 0;

    // first scan for counting and idmapping
    while (NULL != fgets(buffer, LR_LINE_LEN, fp)) {
        str_array = split(trim(buffer, 3), '\t', &count);
        if (count < 3){
            goto free_str;
        }
        int tlen = atoi(str_array[1]);
        if (1 == lr->p.binary && count != (tlen + 2)){
            goto free_str;
        }
        if (0 == lr->p.binary && count != ((tlen + 1) << 1)){
            goto free_str;
        }
        int rtlen = 0;
        for (int i = 0; i < tlen; i++){
            int id = 0;
            if (1 == lr->p.binary) id = i + 2;
            else id = (i << 1) + 2;
            if (-1 != idmap_get_value(idmap, str_array[id])) {
                rtlen += 1;
            }
        }
        if (rtlen > 0){
            tf_cnt += rtlen;
            row += 1;
        }
free_str:
        free(str_array[0]);
        free(str_array);
    }
    lr->test_ds->r   = row;
    lr->test_ds->y   = (double*)malloc(sizeof(double) * row);
    lr->test_ds->l   = (int*)malloc(sizeof(int) * row);
    lr->test_ds->ids = (int*)malloc(sizeof(int) * tf_cnt);
    if (0 == lr->p.binary){
        lr->test_ds->val = (double*)malloc(sizeof(double) * tf_cnt);
    }

    // second scan for loading the data to malloced space
    rewind(fp);
    tf_cnt = row = 0;
    while (NULL != fgets(buffer, LR_LINE_LEN, fp)){
        str_array = split(trim(buffer, 3), '\t', &count);
        if (count < 3) {
            goto str_free;
        }
        int tlen = atoi(str_array[1]);
        if (1 == lr->p.binary && count != (tlen + 2)){
            goto str_free;
        }
        if (0 == lr->p.binary && count != ((tlen + 1) << 1)) {
            goto str_free;
        }
        int rtlen = 0;
        for (int i = 0; i < tlen; i++){
            int id = 0;
            if (1 == lr->p.binary) id = i + 2;
            else id = (i << 1) + 2;
            int iid = idmap_get_value(idmap, str_array[id]);
            if (iid != -1){
                lr->test_ds->ids[tf_cnt] = iid;
                if (0 == lr->p.binary){
                    lr->test_ds->val[tf_cnt] = atof(str_array[id + 1]);
                }
                tf_cnt += 1;
                rtlen  += 1;
            }
        }
        if (rtlen > 0) {
            lr->test_ds->l[row] = rtlen;
            lr->test_ds->y[row] = atof(str_array[0]);
            row += 1;
        }
str_free:
        free(str_array[0]);
        free(str_array);
    }
    fclose(fp);
}

static void free_ds(LRDS * ds){
    if (ds){
        if (ds->l){
            free(ds->l);
            ds->l = NULL;
        }
        if (ds->y){
            free(ds->y);
            ds->y = NULL;
        }
        if (ds->ids){
            free(ds->ids);
            ds->ids = NULL;
        }
        if (ds->val){
            free(ds->val);
            ds->val = NULL;
        }
        free(ds);
    }
}

/* ----------------------------------------
 * brief : LR gradient function
 * x     : current theta learned
 * _ds   : dataset for LR learn
 * g     : gradient vector on current theta
 * return: current LR loss function value
 * ---------------------------------------- */
void lr_grad(double *x, void *_ds, double *g) {
    LR * lr = (LR*) _ds;
    double yest = 0.0, hx = 0.0;
    double *val = lr->train_ds->val;
    double *y   = lr->train_ds->y;
    int    *id  = lr->train_ds->ids;
    int    *len = lr->train_ds->l;
    int     col = lr->c;
    int     row = lr->train_ds->r;
    int i = 0, j = 0, offs = 0;
    memset(g, 0, sizeof(double) * col);
    for (offs = i = 0; i < row; i++) {
        yest = 0.0;
        if (val) {
            for (j = 0; j < len[i]; j++) {
                yest += val[offs + j] * x[id[offs + j]];
            }
        } else {
            for (j = 0; j < len[i]; j++) {
                yest += x[id[offs + j]];
            }
        }
        if (yest < -30) {
            hx = 0.0;
        } else if (yest > 30) {
            hx = 1.0;
        } else {
            hx = 1.0 / (1.0 + exp(-yest));
        }
        if (val) {
            for (j = 0; j < len[i]; j++) {
                g[id[offs + j]] += (hx - y[i]) * val[offs + j];
            }
        } else {
            for (j = 0; j < len[i]; j++) {
                g[id[offs + j]] += (hx - y[i]);
            }
        }
        offs += len[i];
    }
    // Just for L2 Norm 
    if (lr->p.method == 2){
        for (i = 0; i < col; i++){
            g[i] += lr->p.lambda * (x[i] + x[i]);
        }
    }
}

/* ----------------------------------------
 * brief : LR loss function value 
 * x     : current theta learned
 * _ds   : dataset for LR learn
 * return: current LR loss function value
 * ---------------------------------------- */
double lr_eval(double *x, void *_ds) {
    LR * lr = (LR *) _ds;
    double loss = 1.0, yest = 0.0, add = 0.0, regloss = 0.0;
    double *val = lr->train_ds->val;
    int    *id  = lr->train_ds->ids;
    int    *len = lr->train_ds->l;
    double *y   = lr->train_ds->y;
    int     row = lr->train_ds->r;
    int offs =  0, i = 0, j = 0;

    for (offs = i = 0; i < row; i++) {
        yest = 0.0;
        if (val) {
            for (j = 0; j < len[i]; ++j) {
                yest += val[offs + j] * x[id[offs + j]];
            }
        } else {
            for (j = 0; j < len[i]; ++j) {
                yest += x[id[offs + j]];
            }
        }
        if (yest > 30.0){
            add = yest;
        }
        else if (yest > -30.0){
            add = log(1 + exp(yest));
        }
        else{
            add = 0.0;
        }
        if (y[i] > 0){
            add -= yest;
        }
        loss += add;
        offs += len[i];
    }

    // add loss from regularization
    regloss = 0.0;
    if (lr->p.method == 2){       // for L2 Norm
        for (i = 0; i < lr->c; i++){
            regloss += x[i] * x[i];
        }
        loss += regloss * lr->p.lambda;
    }
    else if (lr->p.method == 1){  // for L1 Norm
        for (i = 0; i < lr->c; i++){
            if (x[i] > 0.0){
                regloss += x[i];
            }
            else if (x[i] < 0.0){
                regloss -= x[i];
            }
        }
        loss += regloss * lr->p.lambda;
    }
    return loss;
}

double lr_auc(double *x, void *_ds){
    LRDS * ds = (LRDS*)_ds;
    double  *y  = ds->y;
    int     row = ds->r;
    double *val = ds->val;
    int    *id  = ds->ids;
    int    *len = ds->l;
    double * s = (double*)malloc(sizeof(double) * row);
    memset(s, 0, sizeof(double) * row);
    int offs =  0, i = 0, j = 0;
    for (i = offs = 0; i < row; i++){
        if (val){
            for (j = 0; j < len[i]; j++){
                s[i] += val[offs + j] * x[id[offs + j]]; 
            }
        }
        else{
            for (j = 0; j < len[i]; j++){
                s[i] += x[id[offs + j]]; 
            }
        }
        offs += len[i];
    }
    double auc_v = auc(ds->r, s, y);
    free(s); s = NULL;
    return auc_v;
}

int lr_repo(double *x0, double *x1, void *_ds) {
    LR * lr = (LR *)_ds;
    double val1 = lr_eval(x0, _ds);
    double val2 = lr_eval(x1, _ds);
    if (fabs(val2 - val1) < 1e-5){
        fprintf(stderr, "conv done exit\n");
        return 1;
    }
    int i = ++lr->p.iterno;
    fprintf(stderr, "iter: %4d, loss: %.10f", i, val2);
    if (i % lr->p.savestep == 0){
        double auc = lr_auc(x1, lr->train_ds);
        fprintf(stderr, ",train_auc: %.10f", auc);
        if (lr->test_ds){
            auc = lr_auc(x1, lr->test_ds);
            fprintf(stderr, ",test_auc: %.10f", auc);
        }
        memmove(lr->x, x1, sizeof(double) * lr->c);
        save_lr(lr, i);
    }
    fprintf(stderr, "\n");
    return 0;
}
 
LR * create_lr_model(){
    LR * lr = (LR*)malloc(sizeof(LR));
    memset(lr, 0, sizeof(LR));
    return lr;
}

int   init_lr(LR * lr){
    IdMap * idmap = idmap_create();
    if (0 != load_train_ds(lr, idmap)) {
        idmap_free(idmap); idmap = NULL;
        return -1;
    }
    if (NULL != lr->p.te_file) {
        load_test_ds(lr, idmap);
    }
    idmap_free(idmap); idmap = NULL;
    return 0;
}

int  learn_lr(LR * lr){
    if (lr->p.method == 2){
        lbfgs(lr, lr_eval, lr_grad, lr_repo, 5, lr->c, lr->p.niters, lr->x);
    }
    else if (lr->p.method == 1){
        owlqn(lr, lr_eval, lr_grad, lr_repo, 5, lr->c, lr->p.niters, lr->p.lambda, lr->x);
    }
    return 0;
}

void save_lr(LR * lr, int n){
    FILE * fp = NULL;
    char out_file[512];
    if (n < lr->p.niters){
        sprintf(out_file, "%s/%d_coe", lr->p.out_dir, n);
    }
    else {
        sprintf(out_file, "%s/%s_coe", lr->p.out_dir, "f");
    }
    if (NULL == (fp = fopen(out_file, "w"))){
        fprintf(stderr, "can not write to file \"%s\"\n", out_file);
        return;
    }
    for (int i = 0; i < lr->c; i++){
        fprintf(fp, "%s\t%.10f\n", lr->id_map[i], lr->x[i]);
    }
    fclose(fp);
}

void free_lr(LR * lr){
    if (lr->train_ds){
        free_ds(lr->train_ds);
        lr->train_ds = NULL;
    }
    if (lr->test_ds){
        free_ds(lr->test_ds);
        lr->test_ds = NULL;
    }
    if (lr->x){
        free(lr->x);
        lr->x = NULL;
    }
    if (lr->id_map){
        free(lr->id_map);
        lr->id_map = NULL;
    }
    free(lr);
}


