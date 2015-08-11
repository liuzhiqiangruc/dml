#include <stdio.h>
#include <stdlib.h>
#include "auc.h"
void help() {
    fprintf(stderr, "\nauc usage:        \n");
    fprintf(stderr, "\n./auc filename<string> \n");
}
int main(int argc, char * argv[]){
    if (argc != 2) {
        fprintf(stderr, "command line not well formatted\n");
        help();
        return -1;
    }
    char * filename = argv[1];
    FILE * f = NULL;
    char buffer[1000];
    int pos, n = 0;
    double aucres;
    double *x, *y;
    if (NULL == (f = fopen(filename,"r"))){
        fprintf(stderr,"file does not exist\n");
        return -1;
    }
    while(fgets(buffer, 1000, f) != NULL){
        n++;
    }
    x = (double *)malloc(sizeof(double) * n);
    y = (double *)malloc(sizeof(double) * n);
    pos = 0;
    rewind(f);
    while(fscanf(f, "%lf %lf", &x[pos], &y[pos]) != EOF) {
        pos += 1;
    }
    aucres = auc(n, x, y);
    printf("%lf\n", aucres);
    free(x);
    free(y);
    return 0;
}