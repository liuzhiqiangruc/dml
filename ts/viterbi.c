/* ========================================================
 *   Copyright (C) 2014 All rights reserved.
 *   
 *   filename : viterbi.c
 *   author   : liuzhiqiang01@baidu.com
 *   date     : 2014-12-19
 *   info     : implementation of viterbi
 * ======================================================== */

#include <math.h>
#include <string.h>
#include <stdlib.h>
#include "viterbi.h"

/* ****************************************
 * @param l   : input 0|1 vector 
 * @param o   : output[smoothed] 0|1 vector
 * @param n   : vector length
 * @param r_t : rate for stat <--> stat
 * @param r_o : rate for stat <--> observ
 * ****************************************/
int viterbi(int * l, int * o, int n, int r_t, int r_o){
    // check params 
    if (!l || !o || r_t < 1 || r_o < 1){
        // return fail
        return -1;
    }

    int i = 0;
    double r_t_ = log(r_t);
    double r_o_ = log(r_o);
    double ls0 = 0.0, ls1 = 0.0, cs0 = 0.0, cs1 = 0.0;
    double ps0 = 0.0, ps1 = 0.0;

    // prev_state for trace back
    int (*pre_s)[2] = (int(*)[2])malloc(sizeof(int[2]) * n);

    // init pre_state and output vector
    memset(pre_s, 0, sizeof(int[2]) * n);
    memset(o, 0, sizeof(int) * n);

    // init the begin stat distribute
    ls0 = (l[0] == 0 ? r_o_ : 0.0);
    ls1 = (l[0] == 1 ? r_o_ : 0.0);

    // pass through to calculate max prob
    for (i = 1; i < n ; i++){
        ps0 = ls0 + r_t_ + (l[i] == 0 ? r_o_ : 0.0);
        ps1 = ls1 +        (l[i] == 0 ? r_o_ : 0.0);
        if (ps0 >= ps1){
            cs0 = ps0; pre_s[i][0] = 0;
        }
        else{
            cs0 = ps1; pre_s[i][0] = 1;
        }

        ps0 = ls0 +        (l[i] == 1 ? r_o_ : 0.0);
        ps1 = ls1 + r_t_ + (l[i] == 1 ? r_o_ : 0.0);
        if (ps0 >= ps1){
            cs1 = ps0; pre_s[i][1] = 0;
        }
        else{
            cs1 = ps1; pre_s[i][1] = 1;
        }
        ls0 = cs0; ls1 = cs1;
    }

    // end state with the max prob
    o[n - 1] = cs0 >= cs1 ? 0 : 1;

    // trace back for prev state
    for (i = n - 2; i >= 0; i--){
        o[i] = pre_s[i + 1][o[i + 1]];
    }

    // free the trace pre states
    free(pre_s);
    pre_s = NULL;
    
    // return success
    return 0;
}



////int main(){
////    int a[15] = {0,0,1,0,0,1,1,1,0,1,1,0,0,0,0};
////    int b[15] = {0,0,1,0,0,1,1,1,0,1,1,0,0,0,0};
////    int c = viterbi(a,b,15,10,5);
////    int i = 0;
////    printf("before smooth:\n");
////    for (; i < 15; i++){
////        printf("%d ", a[i]);
////    }
////    printf("\n");
////    printf("after smooth:\n");
////    for (i = 0; i < 15; i++){
////        printf("%d ", b[i]);
////    }
////    printf("\n");
////}
