#include "auc.h"
#include <stdlib.h>
#include <stdio.h>
typedef struct _aucP {
    double x;
    int id;
}AucP;
int cmp(const void *_a, const void *_b) {
    AucP *a = (AucP *) _a;
    AucP *b = (AucP *) _b;
    double l = a->x - b->x;
    return l > 0 ? 1 : (l < 0 ? -1 : 0);
}
void tiedrank(int n, AucP *aucp, double *rk) {
    double curval;
    int i, j, lastpos;
    qsort(aucp, n, sizeof(aucp[0]), cmp);
    curval = aucp[0].x, lastpos = 0;
    for (i = 0; i < n; ++i) {
        if (curval > aucp[i].x + 1e-9 || curval < aucp[i].x - 1e-9) {
            for (j = lastpos; j < i; ++j) {
                rk[aucp[j].id] = (double)(lastpos + i + 1) / 2.;
            }
            lastpos = i;
            curval = aucp[i].x;
        }
    }
    for (j = lastpos; j < i; ++j) {
        rk[aucp[j].id] = (double)(lastpos + i + 1) / 2.;
    }
}
/*
 * x : score
 * y : predict
 */
double auc(int n, double *x, double *y) {
    double *rk = (double*) malloc(sizeof(double) * n);
    AucP *aucp = (AucP *)malloc(sizeof(AucP) * n);
    int i, tsum;
    double rksum, auc;
    for (i = 0; i < n; ++i) {
        aucp[i].x = x[i];
        aucp[i].id = i;
    }
    tiedrank(n, aucp, rk);
    for (rksum = 0., tsum = 0, i = 0; i < n; ++i) {
        if (y[i] >= 1. - 1e-10) {
            rksum += rk[i];
            tsum += 1;
        }
    }
    auc = (rksum - (tsum * (tsum + 1)) / 2) / ((n - tsum) * tsum);
    free(rk);
    free(aucp);
    return auc;
}

//int main(int argc, char * argv[]){
//    char * filename = argv[1];
//    FILE * f = NULL;
//    char buffer[100];
//    int pos, n = 0;
//    double aucres;
//    if (NULL == (f = fopen(filename,"r"))){
//        fprintf(stderr,"file does not exist\n");
//        return -1;
//    }
//    while(fgets(buffer, 100, f) != NULL){
//        n++;
//    }
//    double *x = (double *)malloc(sizeof(double) * n);
//    double *y = (double *)malloc(sizeof(double) * n);
//    pos = 0;
//    rewind(f);
//    while(fscanf(f, "%lf %lf", &x[pos], &y[pos]) != EOF) {
//        pos += 1;
//    }
//    aucres = auc(n, x, y);
//    printf("%lf\n", aucres);
//    free(x);
//    free(y);
//    return 0;
//}