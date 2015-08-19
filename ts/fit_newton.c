/* ========================================================
 *   Copyright (C) 2014 All rights reserved.
 *   
 *   filename : fit_newton.c
 *   author   : liuzhiqiang01@baidu.com
 *              lizeming@baidu.com
 *   date     : 2014-12-24
 *   info     : implement for time series fit
 * ======================================================== */

#include <math.h>
#include <stdlib.h>
#include "newton_opt.h"
#include "fit_newton.h"

typedef struct {
    int n;           //number of point
    double *x;
    double lambda;
} DataSet;

DataSet * dataset_create(int n) {
    DataSet *dataSet = (DataSet *) malloc(sizeof(DataSet));
    dataSet->x = (double *)malloc(sizeof(double) * n);
    dataSet->n = n;
    dataSet->lambda = 0;
    return dataSet;
}

void dataset_free(DataSet *data) {
    if (data != NULL) {
        if (data->x != NULL) {
            free(data->x);
            data->x = NULL;
        }
        free(data);
    }
}

double l2eval(double *x, void *_data) {
    DataSet *dataSet = (DataSet *)_data;
    double ret = 0;
    int i;
    double add;
    for (i = 0; i < dataSet->n; ++i) {
        ret += (dataSet->x[i] - x[i]) * (dataSet->x[i] - x[i]);
    }
    for (add = 0., i = 1; i < dataSet->n; ++i) {
        add += (x[i] - x[i - 1]) * (x[i] - x[i - 1]);
    }
    ret += add * dataSet->lambda;
    return ret / 2.;
}
void l2grad(double *x, void *_data, double *g) {
    DataSet *data = (DataSet *)_data;
    int i;
    for (i = 0; i < data->n; ++i) {
        g[i] = x[i] - data->x[i];
        if (i != 0) {
            g[i] += data->lambda * (x[i] - x[i - 1]);
        }
        if (i != data->n - 1) {
            g[i] -= data->lambda * (x[i + 1] - x[i]);
        }
    }
}

double l1eval(double *x, void *_data) {
    DataSet *data = (DataSet *)_data;
    double ret = 0.;
    double xi = x[0];
    ret += (x[0] - data->x[0]) * (x[0] - data->x[0]);
    for (int i = 1; i < data->n; ++i) {
        xi += x[i];
        ret += (data->x[i] - xi) * (data->x[i] - xi);
    }
    double add = 0.0;
    for (int i = 1; i < data->n; i++){
        add += fabs(x[i]);
    }
    ret += add * data->lambda;
    return ret;

}

void l1grad(double *x, void *_data, double *g) {
    DataSet *data = (DataSet *)_data;
    int i, k, n;
    double add, sum, del;
    for (sum = 0., del = 0., n = data->n, i = 0; i < n; ++i) {
        g[i] = add = 0.;
        for (k = 0; k < n; ++k) {
            add += x[k];
            g[i] += add - data->x[k];
        }
        if (i != 0) {
            sum += x[i - 1];
            del += sum - data->x[i - 1];
            g[i] -= del;
        }
        g[i] *= 2.;
    }
    if (x[0] > 0) {
        g[0] -= data->lambda;
    }
    else if (x[0] < 0){
        g[0] += data->lambda;
    }
}

int l1_repo(double *x0, double *x1, void *_ds) {
    double val1 = l1eval(x0, _ds);
    double val2 = l1eval(x1, _ds);
    if (val2 - val1 < 1e-15){
        return 1;
    }
    return 0;
}

int l2_repo(double *x0, double *x1, void *_ds) {
    double val1 = l2eval(x0, _ds);
    double val2 = l2eval(x1, _ds);
    if (val2 - val1 < 1e-15){
        return 1;
    }
    return 0;
}

void fit_newton(int n, double *datax, double l, int method, int m, int it, double *retx) {
    int i;
    if (datax == NULL || retx == NULL) {
        return;
    }
    DataSet *data = dataset_create(n);
    for (i = 0; i < n; ++i) {
        data->x[i] = datax[i];
    }
    data->lambda = l;
    if (method == 1) {
        owlqn(data, l1eval, l1grad, l1_repo, m, n, it, l, retx);
    } else if(method == 2) {
        lbfgs(data, l2eval, l2grad, l2_repo, m, n, it, retx);
    }
    dataset_free(data);
}
