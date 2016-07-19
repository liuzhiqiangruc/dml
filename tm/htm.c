/* ========================================================
 *   Copyright (C) 2016 All rights reserved.
 *   
 *   filename : htm.c
 *   author   : liuzhiqiangruc@126.com
 *   date     : 2016-07-15
 *   info     : 
 * ======================================================== */

#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tm_config_h.h"
#include "hash.h"
#include "str.h"
#include "tm.h"


typedef struct {
    unsigned short prev;
    unsigned short next;
    unsigned int   count;
}ModelEle;

struct _tm{
    unsigned int d;
    unsigned int l;
    unsigned int v;
    unsigned int t;
    char (*id_d_map)[KEY_SIZE];
    char (*id_l_map)[KEY_SIZE];
    char (*id_v_map)[KEY_SIZE];
    int  (*tokens)[6];
    int  (*doc_cnt)[2];
    int   * doc_entry;
    ModelEle * nd;
    ModelEle * nw;
    int   * nkw;
    int   * wl;
    int   * ln;
    TMHConfig * tmc;
};

static void malloc_space(TM * tm){
    int k = tm->tmc->get_k(tm->tmc);
    tm->id_d_map  = (char(*)[KEY_SIZE])calloc(tm->d,   sizeof(char[KEY_SIZE]));
    tm->id_l_map  = (char(*)[KEY_SIZE])calloc(tm->l,   sizeof(char[KEY_SIZE]));
    tm->id_v_map  = (char(*)[KEY_SIZE])calloc(tm->v,   sizeof(char[KEY_SIZE]));
    tm->tokens    = (int(*)[6])        calloc(tm->t,   sizeof(int[6]));
    tm->doc_cnt   = (int(*)[2])        calloc(tm->d,   sizeof(int[2]));
    tm->doc_entry = (int *)            malloc(tm->d  * sizeof(int));
    tm->ln        = (int *)            calloc(tm->l,   sizeof(int));
    tm->nkw       = (int *)            calloc(k + 1,   sizeof(int));
    tm->wl        = (int *)            calloc(tm->v * tm->l,   sizeof(int));
    tm->nd        = (ModelEle *)       calloc(tm->d * (k + 1), sizeof(ModelEle));
    tm->nw        = (ModelEle *)       calloc(tm->v * (k + 1), sizeof(ModelEle));
    memset(tm->doc_entry, -1, tm->d * sizeof(int));
}

static void fullfill_param(TM * tm){
    int i, d, l, v, x, t, o, p, k;
    k = tm->tmc->get_k(tm->tmc);
    for (i = 0; i < tm->t; i++){
        d = tm->tokens[i][0];
        l = tm->tokens[i][1];
        v = tm->tokens[i][2];
        x = tm->tokens[i][3];
        t = tm->tokens[i][4];
        if (x == 0){
            tm->nd[d * (k + 1) + t].count += 1;
            tm->nw[v * (k + 1) + t].count += 1;
            tm->nkw[t] += 1;
            tm->doc_cnt[d][0] += 1;
        }
        else{
            tm->wl[v * tm->l + l] += 1;
            tm->ln[l]             += 1;
            tm->doc_cnt[d][1]     += 1;
        }
    }
    for (d = 0; d < tm->d; d++){
        o = d * (k + 1);
        p = 0;
        for (t = 1; t < k + 1; t++){
            if (tm->nd[o + t].count > 0){
                tm->nd[o + p].next = t;
                tm->nd[o + t].prev = p;
                p = t;
            }
        }
        tm->nd[o + p].next = 0;
        tm->nd[o].prev = p;
    }
    for (v = 0; v < tm->v; v++){
        o = v * (k + 1);
        p = 0;
        for (t = 1; t < k + 1; t++){
            if (tm->nw[o + t].count > 0){
                tm->nw[o + p].next = t;
                tm->nw[o + t].prev = p;
                p = t;
            }
        }
        tm->nw[o + p].next = 0;
        tm->nw[o].prev = p;
    }
    tm_save(tm, 0);
}

static double init_e(TM * tm, double * pse){
    int t, k;
    double e , alpha, beta, ab, vb;
    k = tm->tmc->get_k(tm->tmc);
    alpha = tm->tmc->get_alpha(tm->tmc);
    beta  = tm->tmc->get_beta(tm->tmc);
    ab    = alpha * beta;
    vb    = beta  * tm->v;
    e     = 0.0;
    for (t = 1; t < k + 1; t++){
        pse[t] = ab / (vb + tm->nkw[t]);
        e += pse[t];
    }
    return e;
}

static double update_e(TM * tm, double * pse, int t, int delta){
    double tmp, beta, alpha, ab, vb;
    alpha = tm->tmc->get_alpha(tm->tmc);
    beta  = tm->tmc->get_beta(tm->tmc);
    ab    = alpha * beta;
    vb    = beta  * tm->v;
    tmp   = ab / (vb + tm->nkw[t]);
    tmp  -= ab / (vb + tm->nkw[t] + delta);
    pse[t] += tmp;
    return tmp;
}

static double init_f(TM * tm, double * psd, int d){
    int t, k, o;
    double f, beta, vb;
    k    = tm->tmc->get_k(tm->tmc);
    beta = tm->tmc->get_beta(tm->tmc);
    vb   = beta * tm->v;
    o    = d * (k + 1);
    f    = 0.0;
    t    = tm->nd[o].next;
    while (t){
        psd[t] = beta * tm->nd[o + t].count / (vb + tm->nkw[t]);
        f += psd[t];
        t = tm->nd[o + t].next;
    }
    return f;
}

static double update_f(TM * tm, double * psd, int d, int t, int delta){
    int k, o;
    double beta, vb, tmp;
    k    = tm->tmc->get_k(tm->tmc);
    o    = d * (k + 1);
    beta = tm->tmc->get_beta(tm->tmc);
    vb   = beta * tm->v;
    tmp  = beta * tm->nd[o + t].count / (vb + tm->nkw[t]);
    tmp -= beta * (tm->nd[o + t].count + delta) / (vb + tm->nkw[t] + delta);
    psd[t] += tmp;
    return tmp;
}

static double init_g(TM * tm, double * psv, int d, int v){
    int t, k, doffs, voffs;
    double g, beta, alpha, vb;
    k     = tm->tmc->get_k(tm->tmc);
    beta  = tm->tmc->get_beta(tm->tmc);
    alpha = tm->tmc->get_alpha(tm->tmc);
    vb    = beta * tm->v;
    doffs = d * (k + 1);
    voffs = v * (k + 1);
    g     = 0.0;
    t     = tm->nw[voffs].next;
    while (t){
        psv[t] = (alpha + tm->nd[doffs + t].count) * tm->nw[voffs + t].count / (vb + tm->nkw[t]);
        g += psv[t];
        t = tm->nw[voffs + t].next;
    }
    return g;
}

static void off_tp_k(TM * tm, int d, int v, int t){
    int doffs, voffs, p, n, k;
    k = tm->tmc->get_k(tm->tmc);
    doffs = d * (k + 1);
    voffs = v * (k + 1);
    tm->nd[doffs + t].count -= 1;
    tm->nw[voffs + t].count -= 1;
    tm->nkw[t] -= 1;
    tm->doc_cnt[d][0] -= 1;
    if (tm->nd[doffs + t].count == 0){
        p = tm->nd[doffs + t].prev;
        n = tm->nd[doffs + t].next;
        tm->nd[doffs + p].next = n;
        tm->nd[doffs + n].prev = p;
    }
    if (tm->nw[voffs + t].count == 0){
        p = tm->nw[voffs + t].prev;
        n = tm->nw[voffs + t].next;
        tm->nw[voffs + p].next = n;
        tm->nw[voffs + n].prev = p;
    }
}

static void addon_tp_k(TM * tm, int d, int v, int t){
    int doffs, voffs, k;
    k = tm->tmc->get_k(tm->tmc);
    doffs = d * (k + 1);
    voffs = v * (k + 1);
    if (tm->nd[doffs + t].count == 0){
        tm->nd[doffs + tm->nd[doffs].prev].next = t;
        tm->nd[doffs + t].prev = tm->nd[doffs].prev;
        tm->nd[doffs + t].next = 0;
        tm->nd[doffs].prev = t;
    }
    if (tm->nw[voffs + t].count == 0){
        tm->nw[voffs + tm->nw[voffs].prev].next = t;
        tm->nw[voffs + t].prev = tm->nw[voffs].prev;
        tm->nw[voffs + t].next = 0;
        tm->nw[voffs].prev = t;
    }
    tm->nd[doffs + t].count += 1;
    tm->nw[voffs + t].count += 1;
    tm->nkw[t] += 1;
    tm->doc_cnt[d][0] += 1;
}

static void off_lc_l(TM * tm, int d, int v, int l){
    tm->wl[v * tm->l + l] -= 1;
    tm->ln[l] -= 1;
    tm->doc_cnt[d][1] -= 1;
}

static void addon_lc_l(TM * tm, int d, int v, int l){
    tm->wl[v * tm->l + l] += 1;
    tm->ln[l] += 1;
    tm->doc_cnt[d][1] += 1;
}

static int sample_k(TM * tm, double * pse
                           , double * psd
                           , double * psv
                           , double e
                           , double f
                           , double g
                           , double r
                           , int d
                           , int v){
    double tmp = 0.0;
    int k, doffs, voffs, t;
    k = tm->tmc->get_k(tm->tmc);
    doffs = d * (k + 1);
    voffs = v * (k + 1);
    if (r < e){
        for (t = 1; t < k + 1; t++){
            tmp += pse[t];
            if (tmp > r){
                break;
            }
        }
    }
    else if (r < e + f){
        r -= e;
        t = tm->nd[doffs].next;
        while (t){
            tmp += psd[t];
            if (tmp > r){
                break;
            }
            t = tm->nd[doffs + t].next;
        }
    }
    else {
        r -= (e + f);
        t = tm->nw[voffs].next;
        while (t){
            tmp += psv[t];
            if (tmp > r){
                break;
            }
            t = tm->nw[voffs + t].next;
        }
    }
    return t;
}

static int gibbs_sample(TM * tm){
    double alpha, beta, ta, e, f, g, s, s1, s2, r, tpr, g0, g1;
    int i, k, d, l, v, x, t;
    k = tm->tmc->get_k(tm->tmc);
    double *pse = (double*)calloc(k + 1, sizeof(double));
    double *psd = (double*)calloc(k + 1, sizeof(double));
    double *psv = (double*)calloc(k + 1, sizeof(double));
    alpha = tm->tmc->get_alpha(tm->tmc);
    beta  = tm->tmc->get_beta(tm->tmc);
    ta    = alpha * k;
    e = init_e(tm, pse);
    for (d = 0; d < tm->d; d++){
        f = init_f(tm, psd, d);
        i = tm->doc_entry[d];
        while (-1 != i){
            l = tm->tokens[i][1];
            v = tm->tokens[i][2];
            x = tm->tokens[i][3];
            t = tm->tokens[i][4];
            if (x == 0){
                off_tp_k(tm, d, v, t);
                e += update_e(tm, pse, t, 1);
                f += update_f(tm, psd, d, t, 1);
            }
            else{
                off_lc_l(tm, d, v, l);
            }
            g = init_g(tm, psv, d, v);
            g0 = tm->tmc->get_g0(tm->tmc);
            g1 = tm->tmc->get_g1(tm->tmc);
            tpr = (g0 + tm->doc_cnt[d][0]) \
                / (g0 + tm->doc_cnt[d][0]+ \
                   g1 + tm->doc_cnt[d][1]);
            s1 = (e + f + g) * tpr / (ta + tm->doc_cnt[d][0]);
            s2 = (1.0 - tpr) * (beta + tm->wl[v * tm->l + l]) \
                             / (beta * tm->v + tm->ln[l]);
            s = s1 + s2;
            r = s * (0.1 + rand()) / (0.1 + RAND_MAX);
            if (s1 > r){
                r = r / tpr * (ta + tm->doc_cnt[d][0]);
                t = sample_k(tm, pse, psd, psv, e, f, g, r, d, v);
                if (t == 0){
                    fprintf(stderr, "sample failed\n");
                    return -1;
                }
                addon_tp_k(tm, d, v, t);
                tm->tokens[i][3] = 0;
                tm->tokens[i][4] = t;
                e += update_e(tm, pse, t, -1);
                f += update_f(tm, psd, d, t, -1);
            }
            else{
                addon_lc_l(tm, d, v, l);
                tm->tokens[i][3] = 1;
                tm->tokens[i][4] =-1;
            }
            i = tm->tokens[i][5];
        }
    }
    free(pse); pse = NULL;
    free(psd); psd = NULL;
    free(psv); psv = NULL;
    return 0;
}

TM * tm_create(int argc, char * argv[]){
    TM * tm = (TM*)calloc(1, sizeof(TM));
    tm->tmc = init_h_config();
    if(0 != tm->tmc->set(tm->tmc, argc, argv)){
        tm->tmc->help();
        return NULL;
    }
    return tm;
}

int tm_init(TM * tm){
    FILE * fp = NULL;
    if (NULL == (fp = fopen(tm->tmc->get_d(tm->tmc), "r"))){
        fprintf(stderr, "can not open input file\n");
        return -1;
    }
    char buffer[LDA_LINE_LEN] = {'\0'};
    char *string, *token;
    int tk, d, l, v, x, t;
    Hash * uhs = hash_create(1<<20, STRING);
    Hash * lhs = hash_create(1<<20, STRING);
    Hash * vhs = hash_create(1<<20, STRING);
    tk = 0;
    while (NULL != fgets(buffer, LDA_LINE_LEN, fp)){
        string = trim(buffer, 3);
        token = strsep(&string, "\t");
        hash_add(uhs, token);
        token = strsep(&string, "\t");
        hash_add(lhs, token);
        token = strsep(&string, "\t");
        hash_add(vhs, token);
        tk += 1;
    }
    tm->d = hash_cnt(uhs);
    tm->l = hash_cnt(lhs);
    tm->v = hash_cnt(vhs);
    tm->t = tk;
    tk = 0;
    malloc_space(tm);
    rewind(fp);
    while (NULL != fgets(buffer, LDA_LINE_LEN, fp)){
        string = trim(buffer, 3);
        token = strsep(&string, "\t");
        d = hash_find(uhs, token);
        if (tm->id_d_map[d][0] == '\0'){
            strncpy(tm->id_d_map[d], token, KEY_SIZE - 1);
        }
        tm->tokens[tk][0] = d;
        l = hash_find(lhs, token);
        if (tm->id_l_map[l][0] == '\0'){
            strncpy(tm->id_l_map[l], token, KEY_SIZE - 1);
        }
        tm->tokens[tk][1] = l;
        v = hash_find(vhs, token);
        if(tm->id_v_map[v][0] == '\0'){
            strncpy(tm->id_v_map[v], token, KEY_SIZE - 1);
        }
        tm->tokens[tk][2] = v;
        x = 0;
        token = strsep(&string, "\t");
        if (token){
            x = atoi(token);
        }
        tm->tokens[tk][3] = x;
        t = (int) ((0.1 + rand()) / (0.1 + RAND_MAX) * tm->tmc->get_k(tm->tmc));
        t += 1;
        if (x == 1){
            t = -1;
        }
        else if (token){
            token = strsep(&string, "\t");
            t = atoi(token);
        }
        tm->tokens[tk][4] = t;
        tm->tokens[tk][5] = tm->doc_entry[d];
        tm->doc_entry[d] = tk;
        tk += 1;
    }
    fclose(fp);
    hash_free(uhs); uhs = NULL;
    hash_free(lhs); lhs = NULL;
    hash_free(vhs); vhs = NULL;
    return 0;
}

void tm_est(TM * tm){
    int n;
    long sec1, sec2;
    fullfill_param(tm);
    for (n = 1; n < tm->tmc->get_n(tm->tmc) + 1; n++){
        sec1 = time(NULL);
        gibbs_sample(tm);
        sec2 = time(NULL);
        fprintf(stderr, "iter %d done, using %ld seconds\n", n, sec2 - sec1);
        if (n % tm->tmc->get_s(tm->tmc) == 0){
            tm_save(tm, n);
        }
    }
}

void tm_save(TM * tm, int n){
    int d, l, v, t, k, o;
    FILE *fp = NULL;
    char nd_file[512];
    char nw_file[512];
    char tk_file[512];
    char wl_file[512];
    char * out_dir = tm->tmc->get_o(tm->tmc);
    k = tm->tmc->get_k(tm->tmc);
    if (n < tm->tmc->get_n(tm->tmc) && n >= 0){
        sprintf(nd_file,  "%s/%d_doc_topic",  out_dir, n);
        sprintf(nw_file,  "%s/%d_word_topic", out_dir, n);
        sprintf(tk_file,  "%s/%d_token_topic",out_dir, n);
        sprintf(wl_file,  "%s/%d_word_local", out_dir, n);
    }
    else{
        sprintf(nd_file,  "%s/%s_doc_topic",  out_dir, "f");
        sprintf(nw_file,  "%s/%s_word_topic", out_dir, "f");
        sprintf(tk_file,  "%s/%s_token_topic",out_dir, "f");
        sprintf(wl_file,  "%s/%s_word_local", out_dir, "f");
    }
    if (NULL == (fp = fopen(nd_file, "w"))) {
        fprintf(stderr, "can not open file \"%s\"", nd_file);
        return;
    }
    for (d = 0; d < tm->d; d++) {
        fprintf(fp, "%s", tm->id_d_map[d]);
        o = d * (k + 1);
        for (t = 1; t < k + 1; t++){
            fprintf(fp, "\t%d", tm->nd[o + t].count);
        }
        fprintf(fp, "\n");
    }
    fclose(fp);
    if (NULL == (fp = fopen(nw_file, "w"))) {
        fprintf(stderr, "can not open file \"%s\"", nw_file);
        return;
    }
    for (v = 0; v < tm->v; v++) {
        fprintf(fp, "%s", tm->id_v_map[v]);
        o = v * (k + 1);
        for (t = 1; t < k + 1; t++){
            fprintf(fp, "\t%d", tm->nw[o + t].count);
        }
        fprintf(fp, "\n");
    }
    fclose(fp);
    if (NULL == (fp = fopen(tk_file, "w"))) {
        fprintf(stderr, "can not open file \"%s\"", tk_file);
        return;
    }
    for (t = 0; t < tm->t; t++) {
        fprintf(fp, "%s\t%s\t%s\t%d\t%d\n"          \
                  ,  tm->id_d_map[tm->tokens[t][0]] \
                  ,  tm->id_l_map[tm->tokens[t][1]] \
                  ,  tm->id_v_map[tm->tokens[t][2]] \
                  ,  tm->tokens[t][3]
                  ,  tm->tokens[t][4]);
    }
    fclose(fp);
    if (NULL == (fp = fopen(wl_file, "w"))){
        fprintf(stderr, "can not open file \"%s\"", wl_file);
        return;
    }
    fprintf(fp, "word:local");
    for (l = 0; l < tm->l; l++){
        fprintf(fp, "\t%s", tm->id_l_map[l]);
    }
    fprintf(fp, "\n");
    for (v = 0; v < tm->v; v++){
        fprintf(fp, "%s", tm->id_v_map[v]);
        for (l = 0; l < tm->l; l++){
            fprintf(fp, "\t%d", tm->wl[v * tm->l + l]);
        }
        fprintf(fp, "\n");
    }
    fclose(fp);
}

void tm_free(TM * tm){
    if (tm){
        if (tm->id_d_map){
            free(tm->id_d_map);
            tm->id_d_map = NULL;
        }
        if (tm->id_l_map){
            free(tm->id_l_map);
            tm->id_l_map = NULL;
        }
        if (tm->id_v_map){
            free(tm->id_v_map);
            tm->id_v_map = NULL;
        }
        if (tm->tokens){
            free(tm->tokens);
            tm->tokens = NULL;
        }
        if (tm->doc_cnt){
            free(tm->doc_cnt);
            tm->doc_cnt = NULL;
        }
        if (tm->doc_entry){
            free(tm->doc_entry);
            tm->doc_entry = NULL;
        }
        if (tm->nd){
            free(tm->nd);
            tm->nd = NULL;
        }
        if (tm->nw){
            free(tm->nw);
            tm->nw = NULL;
        }
        if (tm->nkw){
            free(tm->nkw);
            tm->nkw = NULL;
        }
        if (tm->wl){
            free(tm->wl);
            tm->wl = NULL;
        }
        if (tm->ln){
            free(tm->ln);
            tm->ln = NULL;
        }
        if (tm->tmc){
            tm->tmc->free(tm->tmc);
            tm->tmc = NULL;
        }
    }
    free(tm);
}
