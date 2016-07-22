/* ========================================================
 *   Copyright (C) 2016 All rights reserved.
 *   
 *   filename : xtm.c
 *   author   : liuzhiqiangruc@126.com
 *   date     : 2016-07-19
 *   info     : 
 * ======================================================== */
#include <math.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tm_config_x.h"
#include "hash.h"
#include "str.h"
#include "tm.h"

struct _tm {
    unsigned int d;
    unsigned int v;
    unsigned int t;
    char (*id_d_map)[KEY_SIZE];
    char (*id_v_map)[KEY_SIZE];
    double  gdisp[11];
    double (*disp)[11];
    double (*xy)[2];
    int  (*tokens)[4];
    int  (*doc_cnt)[2];
    int   *nd;
    int   *nw;
    int   *nkw;
    int   *wl;
    int   *ln; 
    int   sln;
    int   gln;
    TMXConfig * tmc;
};

static void malloc_space(TM * tm){
    int k, l;
    k = tm->tmc->get_k(tm->tmc);
    l = tm->tmc->get_l(tm->tmc);
    tm->id_d_map = (char(*)[KEY_SIZE])calloc(tm->d, sizeof(char[KEY_SIZE]));
    tm->id_v_map = (char(*)[KEY_SIZE])calloc(tm->v, sizeof(char[KEY_SIZE]));
    tm->xy       = (double(*)[2])     calloc(tm->t, sizeof(double[2]));
    tm->tokens   = (int(*)[4])        calloc(tm->t, sizeof(int[4]));
    tm->doc_cnt  = (int(*)[2])        calloc(tm->d, sizeof(int[2]));
    tm->nd       = (int *)            calloc(tm->d * k, sizeof(int));
    tm->nw       = (int *)            calloc(tm->v * k, sizeof(int));
    tm->wl       = (int *)            calloc(tm->v * l, sizeof(int));
    tm->nkw      = (int *)            calloc(k,     sizeof(int));
    tm->ln       = (int *)            calloc(l,     sizeof(int));
    tm->disp     = (double(*)[11])    calloc(l,     sizeof(double[11]));
}

static void update_g_param(TM * tm){
    double mx, my, xx, yy, xy, det, invxx, invyy, invxy;
    mx            = tm->gdisp[0] / tm->gln;
    my            = tm->gdisp[1] / tm->gln;
    xx            = tm->gdisp[2] / tm->gln - mx * mx;
    yy            = tm->gdisp[3] / tm->gln - my * my;
    xy            = tm->gdisp[4] / tm->gln - mx * my;
    det           = xx * yy - xy * xy;
    invxx         = yy / det;
    invyy         = xx / det;
    invxy         = -xy / det;
    tm->gdisp[5]  = mx;
    tm->gdisp[6]  = my;
    tm->gdisp[7]  = det;
    tm->gdisp[8]  = invxx;
    tm->gdisp[9]  = invyy;
    tm->gdisp[10] = invxy;
}

static void update_l_param(TM * tm){
    double mx, my, xx, yy, xy, det, invxx, invyy, invxy;
    int t, l;
    l = tm->tmc->get_l(tm->tmc);
    for (t = 0; t < l; t++){
        mx              = tm->disp[t][0] / tm->ln[t];
        my              = tm->disp[t][1] / tm->ln[t];
        xx              = tm->disp[t][2] / tm->ln[t] - mx * mx;
        yy              = tm->disp[t][3] / tm->ln[t] - my * my;
        xy              = tm->disp[t][4] / tm->ln[t] - mx * my;
        det             = xx * yy - xy * xy;
        invxx           = yy / det;
        invyy           = xx / det;
        invxy           = -xy / det;
        tm->disp[t][5]  = mx;
        tm->disp[t][6]  = my;
        tm->disp[t][7]  = det;
        tm->disp[t][8]  = invxx;
        tm->disp[t][9]  = invyy;
        tm->disp[t][10] = invxy;
    }
}

static void fullfill_param(TM * tm){
    int i, d, v, t, x, k, l;
    double lx, ly;
    k = tm->tmc->get_k(tm->tmc);
    l = tm->tmc->get_l(tm->tmc);
    for (i = 0; i < tm->t; i++){
        d  = tm->tokens[i][0];
        v  = tm->tokens[i][1];
        x  = tm->tokens[i][2];
        t  = tm->tokens[i][3];
        lx = tm->xy[i][0];
        ly = tm->xy[i][1];
        if (x == 0){
            tm->nd[d * k + t] += 1;
            tm->nw[v * k + t] += 1;
            tm->nkw[t]        += 1;
            tm->doc_cnt[d][0] += 1;
            tm->gln           += 1;
            tm->gdisp[0]      += lx;
            tm->gdisp[1]      += ly;
            tm->gdisp[2]      += lx * lx;
            tm->gdisp[3]      += ly * ly;
            tm->gdisp[4]      += lx * ly;
        }
        else{
            tm->doc_cnt[d][1] += 1;
            tm->wl[v * l + t] += 1;
            tm->ln[t]         += 1;
            tm->sln           += 1;
            tm->disp[t][0]    += lx;
            tm->disp[t][1]    += ly;
            tm->disp[t][2]    += lx * lx;
            tm->disp[t][3]    += ly * ly;
            tm->disp[t][4]    += lx * ly;
        }
    }
    update_g_param(tm);
    update_l_param(tm);
    tm_save(tm, 0);
}

static void gibbs_sample(TM * tm){
    int i, d, v, x, t, k, l;
    double dx, dy, det, invxx, invyy, invxy;
    double lx, ly, g0, g1, u, alpha, beta, vb, at, rnd;
    double *prb;
    k     = tm->tmc->get_k(tm->tmc);
    l     = tm->tmc->get_l(tm->tmc);
    alpha = tm->tmc->get_alpha(tm->tmc);
    beta  = tm->tmc->get_beta(tm->tmc);
    at    = alpha * k;
    vb    = beta * tm->v;
    g0    = tm->tmc->get_g0(tm->tmc);
    g1    = tm->tmc->get_g1(tm->tmc);
    u     = tm->tmc->get_u(tm->tmc);
    prb = (double *)calloc(k + l, sizeof(double));
    for (i = 0; i < tm->t; i++){
        d  = tm->tokens[i][0];
        v  = tm->tokens[i][1];
        x  = tm->tokens[i][2];
        t  = tm->tokens[i][3];
        lx = tm->xy[i][0];
        ly = tm->xy[i][1];
        if (x == 0){
            tm->nd[d * k + t] -= 1;
            tm->nw[v * k + t] -= 1;
            tm->nkw[t]        -= 1;
            tm->doc_cnt[d][0] -= 1;
        }
        else{
            if (tm->ln[t] < 100){
                continue;
            }
            tm->wl[v * l + t] -= 1;
            tm->ln[t]         -= 1;
            tm->sln           -= 1;
            tm->doc_cnt[d][1] -= 1;
            tm->disp[t][0]    -= lx;
            tm->disp[t][1]    -= ly;
            tm->disp[t][2]    -= lx * lx;
            tm->disp[t][3]    -= ly * ly;
            tm->disp[t][4]    -= lx * ly;
        }
        for(t = 0; t < k; t++){
            prb[t]  = (g0 + tm->doc_cnt[d][0]) \
                    / (g0 + tm->doc_cnt[d][0]+ \
                       g1 + tm->doc_cnt[d][1]) \
                    * (tm->nd[d * k + t] + alpha) \
                    * (tm->nw[v * k + t] + beta)  \
                    / (vb + tm->nkw[t])        \
                    / (at + tm->doc_cnt[d][0]);
            dx      = lx - tm->gdisp[5];
            dy      = ly - tm->gdisp[6];
            det     = tm->gdisp[7];
            invxx   = tm->gdisp[8];
            invyy   = tm->gdisp[9];
            invxy   = tm->gdisp[10];
            prb[t] *= 1.0 / (sqrt(det) * exp(0.5 * (dx * dx * invxx + dy * dy * invyy + 2.0 * dx * dy * invxy)));
            if (t > 0){
                prb[t] += prb[t - 1];
            }
        }
        for (t = 0; t < l; t++){
            prb[t + k] = (g1 + tm->doc_cnt[d][1])   \
                       / (g0 + tm->doc_cnt[d][0]+   \
                          g1 + tm->doc_cnt[d][1])   \
                       * (u + tm->ln[t])            \
                       * (tm->wl[v * l + t] + beta) \
                       / (u * l + tm->sln)          \
                       / (tm->ln[t] + vb);
            dx    = lx - tm->disp[t][5];
            dy    = ly - tm->disp[t][6];
            det   = tm->disp[t][7];
            invxx = tm->disp[t][8];
            invyy = tm->disp[t][9];
            invxy = tm->disp[t][10];
            prb[t + k] *= 1.0 / (sqrt(det) * exp(0.5 * (dx * dx * invxx + dy * dy * invyy + 2.0 * dx * dy * invxy)));
            prb[t + k] += prb[t + k - 1];
        }
        rnd = prb[l + k - 1] * rand() / (1.0 + RAND_MAX);
        for (t = 0; t < k + l; t++){
            if (prb[t] > rnd){
                break;
            }
        }
        if (t == k + l){
            fprintf(stderr, "+");
            t = tm->tokens[i][3];
            if (x == 1){
                t += k;
            }
        }
        if (t < k){
            tm->nd[d * k + t] += 1;
            tm->nw[v * k + t] += 1;
            tm->nkw[t]        += 1;
            tm->doc_cnt[d][0] += 1;
            tm->tokens[i][2]   = 0;
            tm->tokens[i][3]   = t;
        }
        else if (t < k + l){
            t -= k;
            tm->wl[v * l + t] += 1;
            tm->ln[t]         += 1;
            tm->sln           += 1;
            tm->doc_cnt[d][1] += 1;
            tm->disp[t][0]    += lx;
            tm->disp[t][1]    += ly;
            tm->disp[t][2]    += lx * lx;
            tm->disp[t][3]    += ly * ly;
            tm->disp[t][4]    += lx * ly;
            tm->tokens[i][2]   = 1;
            tm->tokens[i][3]   = t;
        }
    }
    free(prb); prb = NULL;
}

TM * tm_create(int argc, char * argv[]){
    TM * tm = (TM*)calloc(1, sizeof(TM));
    tm->tmc = init_x_config();
    if (0 != tm->tmc->set(tm->tmc, argc, argv)){
        tm->tmc->help();
        return NULL;
    }
    return tm;
}

int tm_init(TM * tm){
    FILE *fp = NULL;
    char  buffer[LDA_LINE_LEN] = {'\0'};
    char *string, *token;
    int tk, d, v, x, t, k, l;
    double g0, g1, rnd;
    g0 = tm->tmc->get_g0(tm->tmc);
    g1 = tm->tmc->get_g1(tm->tmc);
    k  = tm->tmc->get_k(tm->tmc);
    l  = tm->tmc->get_l(tm->tmc);
    if (NULL == (fp = fopen(tm->tmc->get_d(tm->tmc), "r"))){
        fprintf(stderr, "can not open input file\n");
        return -1;
    }
    Hash * uhs = hash_create(1<<20, STRING);
    Hash * vhs = hash_create(1<<20, STRING);
    tk = 0;
    while(NULL != fgets(buffer, LDA_LINE_LEN, fp)){
        string = trim(buffer, 3);
        token = strsep(&string, "\t");
        hash_add(uhs, token);
        token = strsep(&string, "\t");
        hash_add(vhs, token);
        tk += 1;
    }
    tm->d = hash_cnt(uhs);
    tm->v = hash_cnt(vhs);
    tm->t = tk;
    malloc_space(tm);
    rewind(fp);
    tk = 0;
    while (NULL != fgets(buffer, LDA_LINE_LEN, fp)){
        string = trim(buffer, 3);
        token = strsep(&string, "\t");
        d = hash_find(uhs,token);
        if(tm->id_d_map[d][0] == '\0'){
            strncpy(tm->id_d_map[d], token, KEY_SIZE - 1);
        }
        tm->tokens[tk][0] = d;
        token = strsep(&string, "\t");
        v = hash_find(vhs, token);
        if (tm->id_v_map[v][0] == '\0'){
            strncpy(tm->id_v_map[v], token, KEY_SIZE - 1);
        }
        tm->tokens[tk][1] = v;
        token = strsep(&string, "\t");
        tm->xy[tk][0] = atof(token);
        token = strsep(&string, "\t");
        tm->xy[tk][1] = atof(token);
        rnd = (0.1 + rand()) / (0.1 + RAND_MAX);
        x = 0;
        t = (int)((0.1 + rand()) / (0.1 + RAND_MAX) * k);
        if (rnd > 0.5){
            x = 1;
            t = (int)((0.1 + rand()) / (0.1 + RAND_MAX) * l);
        }
        token = strsep(&string, "\t");
        if (token){
            x = atoi(token);
            token = strsep(&string, "\t");
            t = atoi(token);
        }
        tm->tokens[tk][2] = x;
        tm->tokens[tk][3] = t;
        tk += 1;
    }
    fclose(fp);
    hash_free(uhs); uhs = NULL;
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
        if(n % tm->tmc->get_s(tm->tmc) == 0){
            update_l_param(tm);
            tm_save(tm, n);
        }
    }
}

void tm_save(TM * tm, int n){
    int d, v, t, k, l, o;
    double mx, my, xx, yy, xy, det;
    FILE *fp = NULL;
    char nd_file[512];
    char nw_file[512];
    char wl_file[512];
    char ld_file[512];
    char tk_file[512];
    char * out_dir = NULL;
    k = tm->tmc->get_k(tm->tmc);
    l = tm->tmc->get_l(tm->tmc);
    out_dir = tm->tmc->get_o(tm->tmc);
    if (n < tm->tmc->get_n(tm->tmc) && n >= 0){
        sprintf(nd_file, "%s/%d_doc_topic",    out_dir, n);
        sprintf(nw_file, "%s/%d_word_topic",   out_dir, n);
        sprintf(wl_file, "%s/%d_word_local",   out_dir, n);
        sprintf(ld_file, "%s/%d_local_disp",   out_dir, n);
        sprintf(tk_file, "%s/%d_token_topic",  out_dir, n);
    }
    else{
        sprintf(nd_file, "%s/%s_doc_topic",    out_dir, "f");
        sprintf(nw_file, "%s/%s_word_topic",   out_dir, "f");
        sprintf(wl_file, "%s/%s_word_local",   out_dir, "f");
        sprintf(ld_file, "%s/%s_local_disp",   out_dir, "f");
        sprintf(tk_file, "%s/%s_token_topic",  out_dir, "f");
    }
    if (NULL == (fp = fopen(nd_file, "w"))){
        return;
    }
    for (d = 0; d < tm->d; d++){
        fprintf(fp, "%s", tm->id_d_map[d]);
        o = d * k;
        for (t = 0; t < k; t++){
            fprintf(fp, "\t%d", tm->nd[o + t]);
        }
        fprintf(fp, "\n");
    }
    fclose(fp);
    if (NULL == (fp = fopen(nw_file, "w"))){
        return;
    }
    for (v = 0; v < tm->v; v++){
        o = v * k;
        fprintf(fp, "%s", tm->id_v_map[v]);
        for (t = 0; t < k; t++){
            fprintf(fp, "\t%d", tm->nw[o + t]);
        }
        fprintf(fp, "\n");
    }
    fclose(fp);
    if (NULL == (fp = fopen(wl_file, "w"))){
        return;
    }
    for (v = 0; v < tm->v; v++){
        o = v * l;
        fprintf(fp, "%s", tm->id_v_map[v]);
        for (t = 0; t < l; t++){
            fprintf(fp, "\t%d", tm->wl[o + t]);
        }
        fprintf(fp, "\n");
    }
    fclose(fp);
    if (NULL == (fp = fopen(ld_file, "w"))){
        return;
    }
    for (t = 0; t < l; t++){
        mx = tm->disp[t][5];
        my = tm->disp[t][6];
        xx = tm->disp[t][2] / tm->ln[t] - mx * mx;
        yy = tm->disp[t][3] / tm->ln[t] - my * my;
        xy = tm->disp[t][4] / tm->ln[t] - mx * my;
        det = tm->disp[t][7];
        fprintf(fp,"%.6lf\t%.6lf\t%.6lf\t%.6lf\t%.6lf\t%.6lf\n"
                  , mx, my, xx, yy, xy, det);
    }
    fclose(fp);
    if (NULL == (fp = fopen(tk_file, "w"))){
        return;
    }
    for (t = 0; t < tm->t; t++){
        fprintf(fp, "%s\t%s\t%.6lf\t%.6lf\t%d\t%d\n" \
                   , tm->id_d_map[tm->tokens[t][0]]  \
                   , tm->id_v_map[tm->tokens[t][1]]  \
                   , tm->xy[t][0]                    \
                   , tm->xy[t][1]                    \
                   , tm->tokens[t][2]                \
                   , tm->tokens[t][3]);
    }
    fclose(fp);
}

void tm_free(TM * tm){
    if (tm){
        if (tm->id_d_map){
            free(tm->id_d_map);
            tm->id_d_map = NULL;
        }
        if (tm->id_v_map){
            free(tm->id_v_map);
            tm->id_v_map = NULL;
        }
        if (tm->disp){
            free(tm->disp);
            tm->disp = NULL;
        }
        if (tm->xy){
            free(tm->xy);
            tm->xy = NULL;
        }
        if (tm->tokens){
            free(tm->tokens);
            tm->tokens = NULL;
        }
        if (tm->doc_cnt){
            free(tm->doc_cnt);
            tm->doc_cnt = NULL;
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
        free(tm);
    }
}
