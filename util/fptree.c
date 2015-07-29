/* ========================================================
 *   Copyright (C) 2014 All rights reserved.
 *   
 *   filename : fptree.c
 *   author   : liuzhiqiang01@baidu.com
 *   date     : 2014-07-09
 *   info     : implementation of frequent pattern tree
 * ======================================================== */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "hashtable.h"
#include "str.h"
#include "fptree.h"

#define LINE_LEN  256

typedef struct {
    char key[KEY_SIZE];
    int count;
} ItemCount;

static int itemcompare(const void * arg1, const void * arg2){
    const ItemCount * a1 = (const ItemCount*) arg1;
    const ItemCount * a2 = (const ItemCount*) arg2;
    if (a1->count == a2->count) return 0;
    return a1->count > a2->count ? -1 : 1;
}

static int intcompare(const void * a1, const void * a2){
    return (*(int *)a1 - *(int *)a2);
}

static int _buildHeader(const char* infile, FPTREE * fpt, HashTable * item_dict, char sep){
    FILE * fp = NULL;
    if ((fp = fopen(infile,"r")) == NULL){
        fprintf(stderr,"open input file failed\n");
        return -1;
    }
    char buffer[LINE_LEN];
    char ** str_array = NULL;
    int count = 0, item_count = 0;
    int line = 0;
    HashTable * tmp_dict = create_hashtable(5000000,char*,int);
    while (NULL != fgets(buffer,LINE_LEN,fp)){
        str_array = split(trim(buffer,3), sep ,&count);
        if (count < 3){
            goto str_free;
        }
        int t_count = atoi(str_array[count-1]);
        for (int i = 0; i< count - 1; i++){
            if (strlen(str_array[i]) < 6)
                continue;
            if (NOTEXISTS == hash_exists(tmp_dict,str_array[i])){
                hash_add(tmp_dict,str_array[i],t_count);
            }
            else{
                hash_find(tmp_dict,str_array[i],&item_count);
                item_count += t_count;
                hash_add(tmp_dict,str_array[i],item_count);
            }
        }
str_free:
        free(str_array[0]);
        free(str_array);
        if (++line % 1000000 == 0){
            fprintf(stderr,"load lines %d\n", line);
        }
    }
    fclose(fp);

    fprintf(stderr,"totle words %d\n", hash_num_elements(tmp_dict));

    for (reset(tmp_dict); isnotend(tmp_dict); next(tmp_dict)){
        char * key = skey(tmp_dict);
        int  item_sup = *((int*)value(tmp_dict));
        if (item_sup >= fpt->minsup){
            hash_add(item_dict,key,item_sup);
        }
    }


    hash_free(tmp_dict); tmp_dict = NULL;

    int item_size = 0;
    fpt->headsize = item_size = hash_num_elements(item_dict);
    fpt->header = (FPTLIST *)malloc(sizeof(FPTLIST) * item_size);
    ItemCount * ic = (ItemCount*)malloc(sizeof(ItemCount) * item_size);

    int i = 0;
    for (reset(item_dict); isnotend(item_dict); next(item_dict)){
        char * key = skey(item_dict);
        int item_sup = *((int*)value(item_dict));
        strncpy(ic[i].key,key,KEY_SIZE-1); ic[i].key[KEY_SIZE-1] = '\0';
        ic[i].count = item_sup;
        i += 1;
    }
    reset(item_dict);

    qsort(ic,item_size,sizeof(ItemCount),itemcompare);

    for (int i = 0; i< item_size; i++){
        strncpy(fpt->header[i].itemname,ic[i].key,KEY_SIZE-1);
        fpt->header[i].itemname[KEY_SIZE-1] = '\0';
        fpt->header[i].supp = ic[i].count;
        fpt->header[i].itemlink = 0;
        hash_add(item_dict,ic[i].key,i);
    }
    free(ic);ic = NULL;

    return 0;
}

static void _insert_path(FPTREE * fpt, int items[], int size, int t_count){
    if (!fpt || !items || size <=0 || t_count <= 0){
        return;
    }
    int node = 0, pi = 0, fi = 0;

    for (int i = 0; i < size; i++){
        int item_no = items[i];
        fi = node;
        if (fpt->nodelist[node].ci != 0){
            node = fpt->nodelist[node].ci;
            do{
                pi = node;
                if (fpt->nodelist[node].item == item_no){
                    fpt->nodelist[node].supp += t_count;
                    break;
                }
                node = fpt->nodelist[node].si;
            }while(node != 0);
            if (node == 0){
                fpt->nodelist[pi].si = node = fpt->usindex++;
                fpt->nodelist[node].item = item_no;
                fpt->nodelist[node].supp = t_count;
                fpt->nodelist[node].pi = fi;
                pi = fpt->header[item_no].itemlink;
                fpt->header[item_no].itemlink = node;
                fpt->nodelist[node].li = pi;
            }
        }
        else{
            fpt->nodelist[node].ci = fpt->usindex++;
            node  = fpt->nodelist[node].ci;
            fpt->nodelist[node].item = item_no;
            fpt->nodelist[node].supp = t_count;
            fpt->nodelist[node].pi = fi;
            pi = fpt->header[item_no].itemlink;
            fpt->header[item_no].itemlink = node;
            fpt->nodelist[node].li = pi;
        }
    }
}

static FPTREE * _expend_fpt(FPTREE * fpt){
    unsigned int new_size = 0L;
    new_size = sizeof(FPTREE) + sizeof(FPTNODE) * (fpt->nodespace + INIT_NODE_SIZE);
    FPTREE * p = (FPTREE *)malloc(new_size);
    if (!p) return NULL;
    fprintf(stderr,"new size : %u\n", new_size);
    memset(p,0,new_size);
    memmove(p,fpt,sizeof(FPTREE) + sizeof(FPTNODE) *fpt->nodespace);
    p->nodespace += INIT_NODE_SIZE;
    free(fpt);
    return p;
}


static FPTREE * _construct_fpt(const char * infile, FPTREE * fpt, HashTable * item_dict, char sep){
    FILE * fp = NULL;
    if ((fp = fopen(infile,"r")) == NULL){
        fprintf(stderr,"open input file failed\n");
        return NULL;
    }
    char buffer[LINE_LEN];
    char ** str_array = NULL;
    int items[LINE_LEN];
    int count = 0, item_no = 0, item_size_t = 0, line = 0;

    while (NULL != fgets(buffer,LINE_LEN,fp)){
        item_size_t = 0;
        str_array = split(trim(buffer,3),sep,&count);
        if (count < 3) {
            goto str_free;
        }
        int t_count = atoi(str_array[count-1]);
        for (int i = 0; i< count - 1; i++){
            if (strlen(str_array[i]) < 6)
                continue;
            if (EXISTS == hash_exists(item_dict, str_array[i])){
                hash_find(item_dict,str_array[i], &item_no);
                items[item_size_t++] = item_no;
            }
        }
        if (item_size_t > 1){
            qsort(items, item_size_t, sizeof(items[0]), intcompare);

            if (fpt->usindex + item_size_t >= fpt->nodespace){
                fpt = _expend_fpt(fpt);
                if (!fpt) {
                    fprintf(stderr, "expend fptree failed\n" );
                    return NULL;
                }
                fprintf(stderr, "expend fptree success\n" );
            }
            _insert_path(fpt, items, item_size_t, t_count);
        }

str_free:
        free(str_array[0]);
        free(str_array);

        if (++line % 100000 == 0){
            fprintf(stderr,"%d lines insertion done\n", line);
        }
    }
    fclose(fp);

    return fpt;
}

static void _all_children(int n, FPTREE * fpt, HashTable * id_count_dict){
    int stack[LINE_LEN], i = -1, pitem = -1, pop = 0, psupp = 0, id_count = 0;
    do{
        int ci = fpt->nodelist[n].ci;
        if (ci != 0 && pop == 0){
            pitem = fpt->nodelist[ci].item;
            psupp = fpt->nodelist[ci].supp;
            id_count = 0;
            if (EXISTS == hash_exists(id_count_dict,pitem)){
                hash_find(id_count_dict,pitem,&id_count);
            }
            id_count += psupp;
            hash_add(id_count_dict,pitem,id_count);

            stack[++i] = ci;
            n = ci;
            pop = 0;
        }
        else if (i >= 0){
            n = stack[i--];
            pop = 1;
            int si = fpt->nodelist[n].si;
            if (si != 0){
                pitem = fpt->nodelist[si].item;
                psupp = fpt->nodelist[si].supp;
                id_count = 0;
                if (EXISTS == hash_exists(id_count_dict,pitem)){
                    hash_find(id_count_dict,pitem,&id_count);
                }
                id_count += psupp;
                hash_add(id_count_dict,pitem,id_count);

                stack[++i] = si;
                n = si;
                pop = 0;
            }
        }
    }while(i >= 0);
}

FPTREE * create_fpt(int minsup){
    FPTREE * fpt = (FPTREE*)malloc(sizeof(FPTREE) + sizeof(FPTNODE) * INIT_NODE_SIZE);
    if (!fpt)
        return NULL;
    memset(fpt,0,sizeof(FPTREE) + sizeof(FPTNODE) * INIT_NODE_SIZE);
    fpt->nodespace = INIT_NODE_SIZE;
    fpt->minsup = minsup;
    fpt->nodelist[0].item = -1;
    fpt->usindex = 1;
    return fpt;
}

FPTREE * build_fpt(const char * infile, FPTREE * fpt, char sep){
    HashTable * item_dict = create_hashtable(400000,char*,int);
    if (-1 == _buildHeader(infile, fpt, item_dict, sep)){
        fprintf(stderr,"buildHeader failed\n");
        return NULL;
    }
    fpt = _construct_fpt(infile,fpt,item_dict, sep);
    if (!fpt) return NULL;
    hash_free(item_dict); item_dict = NULL;
    return fpt;
}

int select_ids_by_key(const char * key, FPTREE * fpt, int ids[],int count, int sup_threadhold){
    if (!key || !fpt || !ids || count <=0){
        return -1;
    }
    int i = 0, node_index = -1, out_id_size = 0, link_index = 0;
    int id_count = 0;
    for(; i< fpt->headsize; i++){
        if (strcmp(fpt->header[i].itemname, key) == 0){
            node_index = i;
            break;
        }
    }
    if (node_index == -1){
        return -1;
    }
    link_index = fpt->header[node_index].itemlink;
    if (link_index == 0){
        return -1;
    }
    HashTable * id_count_dict = create_hashtable(count,int,int);
    while(link_index != 0){
        int pindex = fpt->nodelist[link_index].pi;
        int node_supp = fpt->nodelist[link_index].supp;
        while (pindex != 0){
            id_count = 0;
            int pitem = fpt->nodelist[pindex].item;
            if (EXISTS == hash_exists(id_count_dict,pitem)){
                hash_find(id_count_dict,pitem,&id_count);
            }
            id_count += node_supp;
            hash_add(id_count_dict,pitem,id_count);
            pindex = fpt->nodelist[pindex].pi;
        }
        _all_children(link_index,fpt,id_count_dict);
        link_index = fpt->nodelist[link_index].li;
    }
    for (reset(id_count_dict); isnotend(id_count_dict); next(id_count_dict)){
        int pitem = nkey(id_count_dict);
        int pvalue = *((int*)value(id_count_dict));
        if (pvalue >= sup_threadhold){
            ids[out_id_size++] = pitem;
            if (out_id_size >= count)
                break;
        }
    }
    hash_free(id_count_dict); id_count_dict = NULL;
    return out_id_size;
}



void get_sup_by_key(int n, FPTREE * fpt, int ids[], int count, int out_sup[]){
    if (n < 0 || !fpt || !ids || count < 1 || !out_sup){
        return ;
    }
    int node_index = 0, i = 0, pitem = -1;
    node_index = fpt->header[n].itemlink;
    while(node_index != 0){
        int pi = fpt->nodelist[node_index].pi;
        i = 0;
        while(pi != 0 && i < count){
            pitem = fpt->nodelist[pi].item;
            while (i < count && ids[i] > pitem) i += 1;

            if(pitem == ids[i]){
                out_sup[i] += fpt->nodelist[node_index].supp;
                i += 1;
            }
            pi = fpt->nodelist[pi].pi;
        }
        node_index = fpt->nodelist[node_index].li;
    }
}

void print_fpt(FPTREE * fpt){
    // tree info
    printf("%d,%d,%d,%d\n",fpt->minsup,fpt->headsize,fpt->nodespace,fpt->usindex);
    for (unsigned int i = 0; i<= fpt->nodespace; i++){
        printf("%d,%d,%d,%d,%d,%d\n",fpt->nodelist[i].item,fpt->nodelist[i].supp,fpt->nodelist[i].ci,fpt->nodelist[i].si,fpt->nodelist[i].li,fpt->nodelist[i].pi );
    }
    // header
    for(int i = 0; i < fpt->headsize; i++){
        printf("%s\t%d\t%d\n",fpt->header[i].itemname,fpt->header[i].supp,fpt->header[i].itemlink);
    }
}


void free_fpt(FPTREE * fpt){
    if (fpt){
        if (fpt->header){
            free(fpt->header);
            fpt->header = NULL;
        }
        free(fpt);
    }
}

void save_fpt(FPTREE * fpt, const char * serfile){
    FILE * outfp = NULL;
    if (NULL == (outfp = fopen(serfile,"wb"))){
        fprintf(stderr,"can not open the serialize file to save fptree\n");
        return;
    }
    fwrite(fpt,sizeof(FPTREE),1,outfp);
    fwrite(&fpt->nodelist[1],sizeof(FPTNODE),fpt->usindex, outfp);
    fwrite(fpt->header,sizeof(FPTLIST),fpt->headsize,outfp);
    fclose(outfp);
}

FPTREE * load_fpt(const char * serfile){
    FILE * infile = NULL;
    while (NULL == (infile = fopen(serfile,"rb"))){
        fprintf(stderr,"can not open the serialize file to load fptree\n");
        return NULL;
    }
    FPTREE * tmp_fpt = (FPTREE*)malloc(sizeof(FPTREE));
    FPTREE * fpt = NULL;
    fread(tmp_fpt,sizeof(FPTREE),1,infile);
    fpt = (FPTREE*)malloc(sizeof(FPTREE) + sizeof(FPTNODE) * tmp_fpt->usindex);
    memset(fpt,0,sizeof(FPTREE) + sizeof(FPTNODE) * tmp_fpt->usindex);
    memmove(fpt,tmp_fpt,sizeof(FPTREE));
    free(tmp_fpt); tmp_fpt = NULL;


    fread(&fpt->nodelist[1],sizeof(FPTNODE), fpt->usindex, infile);
    fpt->nodespace = fpt->usindex;

    FPTLIST * header = (FPTLIST *)malloc(sizeof(FPTLIST) * fpt->headsize);
    fread(header,sizeof(FPTLIST), fpt->headsize, infile);

    fpt->header = header; header = NULL;

    fclose(infile);
    return fpt;
}
