/* ========================================================
 *   Copyright (C) 2017 All rights reserved.
 *   
 *   filename : regcfg.h
 *   author   : ***
 *   date     : 2017-12-06
 *   info     : 
 * ======================================================== */

#ifndef _REGCFG_H
#define _REGCFG_H

typedef struct {
    int n;                       /* iteration number     */
    int s;                       /* save step            */
    int b;                       /* binary or not        */
    int r;                       /* L1 or L2 norm        */
    int k;                       /* latent dim length    */
    double alpha;                /* learning rate        */
    double gamma;                /* regulization         */
    double toler;                /* tolerance for conv   */
    char * train_input;          /* train input          */
    char * test_input;           /* test  input          */
    char * out_dir;              /* output dir           */
}REGP;



int parse_command_line(REGP *p, int argc, char *argv[]);

#endif //REGCFG_H
