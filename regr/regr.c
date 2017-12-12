/* ========================================================
 *   Copyright (C) 2017 All rights reserved.
 *   
 *   filename : regr.c
 *   author   : liuzhiqiangruc@126.com
 *   date     : 2017-12-06
 *   info     : regression using gradient method
 * ======================================================== */
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include "regr.h"

int init_model(REGR * regr){
    Hash * hs = hash_create(1 << 20, STRING);
    regr->train_ds = data_load(regr->reg_p.train_input, ROW, regr->reg_p.b == 1 ? BINARY : NOBINARY, NO_INITED, hs);
    if (!regr->train_ds){
        hash_free(hs);
        hs = NULL;
        return -1;
    }
    regr->test_ds = data_load(regr->reg_p.test_input, ROW, regr->reg_p.b == 1 ? BINARY : NOBINARY, INITED, hs);
    free(hs);
    hs = NULL;
    regr->feature_len = regr->train_ds->col;
    if (regr->reg_p.k == 0) {   // just for simple lr
        regr->x = (double*)calloc(regr->feature_len, sizeof(double));
    }
    else{                       // with k nodes as latent layer
        regr->x = (double*)calloc((regr->feature_len + 1) * regr->reg_p.k, sizeof(double));
    }
    return 0;
}

REGR * create_model(LEARN_FN learn_fn){
    if (!learn_fn){
        return NULL;
    }
    REGR *regr = (REGR*)calloc(1, sizeof(REGR));
    regr->learn_fn = learn_fn;
    return regr;
}

void save_model(REGR * regr, int n){
    FILE * fp = NULL;
    char * outdir = regr->reg_p.out_dir;
    char out_file[512] = {0};
    int k, c;
    k = regr->reg_p.k;
    c = regr->feature_len;
    mkdir(outdir, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
    if (n < regr->reg_p.n){
        snprintf(out_file, 200, "%s/%d_coe", regr->reg_p.out_dir, n);
    }
    else{
        snprintf(out_file, 200, "%s/f_coe", regr->reg_p.out_dir);
    }
    if (NULL == (fp = fopen(out_file, "w"))){
        fprintf(stderr, "can not open output file");
        return;
    }
    if (k == 0) for (int i = 0; i < c; i++){
        fprintf(fp, "%s\t%.10f\n", regr->train_ds->id_map[i], regr->x[i]);
    }
    else if (k > 0){
        for (int i = 0; i < c; i++) {
            fprintf(fp, "%s" , regr->train_ds->id_map[i]);
            for (int j = 0; j < k; j++){
                fprintf(fp, "\t%.8f", regr->x[i * k + j]);
            }
            fprintf(fp, "\n");
        }
        fprintf(fp, "latent_embeding");
        for (int j = 0; j < k; j++){
            fprintf(fp, "\t%.8f", regr->x[c * k + j]);
        }
        fprintf(fp, "\n");
    }
    fclose(fp);
}

void free_model(REGR * regr){
    if (regr){
        if (regr->train_ds){
            data_free(regr->train_ds);
            regr->train_ds = NULL;
        }
        if (regr->test_ds){
            data_free(regr->test_ds);
            regr->test_ds = NULL;
        }
        if (regr->x){
            free(regr->x);
            regr->x = NULL;
        }
        free(regr);
    }
}
