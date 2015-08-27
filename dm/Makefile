UTIL     = ../util
CC       = gcc -std=c99
CFBASE   = -Wall -pedantic -pipe -Wno-unused-but-set-variable -Wno-unused-variable -Wno-unused-function
INCS     = -I../util
CFLAGS   = $(CFBASE) -DNDEBUG -DNPRINT -O3 -fpic

LD       = gcc
SOFLAGS  = -shared 

OBJS     = kmeans.o     modl.o     lda.o     author_lda.o  bri_lda.o   joint_lda.o  tot.o       totg.o      mf.o  pmf.o   bmf.o   newton.o     regression.o lr.o pr.o gr.o er.o
SOS      = libkmeans.so libmodl.so liblda.so libalda.so    libblda.so  libjlda.so   libtot.so   libtotg.so  libpmf.so libbmf.so   libnewt.so   liblr.so libpr.so libgr.so liber.so
STATICS  = libkmeanss.a libmodls.a libldas.a libaldas.a    libbldas.a  libjldas.a   libtots.a   libtotgs.a  libpmfs.a libbmfs.a   libnewts.a   liblrs.a libprs.a libgrs.a libers.a

util_lda_objs = $(UTIL)/rbtree.o $(UTIL)/set.o $(UTIL)/idmap.o $(UTIL)/str.o
util_pmf_objs = $(util_lda_objs)
util_pr_objs  = $(util_lda_objs) newton.o
util_gr_objs  = $(util_lda_objs) newton.o
util_er_objs  = $(util_lda_objs) newton.o
util_lr_objs  = $(util_pr_objs)  auc.o


all:	$(STATICS)
so:     $(SOS)
objs:   $(OBJS)

## --------- objects --------------

kmeans.o       :   kmeans.h

modl.o         :   modl.h

auc.o          :   auc.h

regression.o   :   regression.c regression.h
	$(CC) $(CFLAGS) $(INCS) -c $< -o $@

lr.o           :   lr.c
	$(CC) $(CFLAGS) $(INCS) -c $< -o $@

pr.o           :   pr.c
	$(CC) $(CFLAGS) $(INCS) -c $< -o $@

gr.o           :   gr.c
	$(CC) $(CFLAGS) $(INCS) -c $< -o $@

er.o           :   er.c
	$(CC) $(CFLAGS) $(INCS) -c $< -o $@

lda.o          :   lda.c lda.h
	$(CC) $(CFLAGS) $(INCS) -c $< -o $@

joint_lda.o    :   joint_lda.c joint_lda.h
	$(CC) $(CFLAGS) $(INCS) -c $< -o $@

bri_lda.o      :   bri_lda.c bri_lda.h
	$(CC) $(CFLAGS) $(INCS) -c $< -o $@

newton.o       :   newton_opt.c newton_opt.h
	$(CC) $(CFLAGS) $(INCS) -c $< -o $@

mf.o           :   mf.c  mf.h
	$(CC) $(CFLAGS) $(INCS) -c $< -o $@

pmf.o          :   pmf.c
	$(CC) $(CFLAGS) $(INCS) -c $< -o $@

bmf.o          :   bmf.c
	$(CC) $(CFLAGS) $(INCS) -c $< -o $@

author_lda.o   :   author_lda.c author_lda.h
	$(CC) $(CFLAGS) -DPHI      $(INCS) -c $< -o $@

tot.o          :   tot.c tot.h
	$(CC) $(CFLAGS) -DBETA     $(INCS) -c $< -o $@

totg.o         :   tot.c tot.h
	$(CC) $(CFLAGS) -DGAUSSIAN $(INCS) -c $< -o $@



## --------- .so --------------

libkmeans.so : kmeans.o
	$(LD) $(SOFLAGS) $< -o $@

libmodl.so : modl.o
	$(LD) $(SOFLAGS) $< -o $@

libnewt.so : newton.o      
	$(LD) $(SOFLAGS) $^ -o $@

liblda.so  : lda.o              $(util_lda_objs)
	$(LD) $(SOFLAGS) $^ -o $@

libalda.so : author_lda.o       $(util_lda_objs)
	$(LD) $(SOFLAGS) $^ -o $@

libjlda.so : joint_lda.o        $(util_lda_objs)
	$(LD) $(SOFLAGS) $^ -o $@

libblda.so : bri_lda.o          $(util_lda_objs)
	$(LD) $(SOFLAGS) $^ -o $@

libtot.so  : tot.o              $(util_lda_objs)
	$(LD) $(SOFLAGS) $^ -o $@

libtotg.so  : totg.o            $(util_lda_objs)
	$(LD) $(SOFLAGS) $^ -o $@

libpmf.so   : pmf.o mf.o        $(util_pmf_objs)
	$(LD) $(SOFLAGS) $^ -o $@

libbmf.so   : bmf.o mf.o        $(util_pmf_objs)
	$(LD) $(SOFLAGS) $^ -o $@

liblr.so    : lr.o regression.o $(util_lr_objs)
	$(LD) $(SOFLAGS) $^ -o $@

libpr.so    : pr.o regression.o $(util_pr_objs)
	$(LD) $(SOFLAGS) $^ -o $@

libgr.so    : gr.o regression.o $(util_gr_objs)
	$(LD) $(SOFLAGS) $^ -o $@

liber.so    : er.o regression.o $(util_er_objs)
	$(LD) $(SOFLAGS) $^ -o $@

## --------- .a --------------

libkmeanss.a : kmeans.o
	ar cr $@ $^

libmodls.a : modl.o
	ar cr $@ $^

libnewts.a : newton.o  
	ar cr $@ $^

liblrs.a   : lr.o regression.o $(util_lr_objs)
	ar cr $@ $^

libprs.a   : pr.o regression.o $(util_pr_objs)
	ar cr $@ $^

libgrs.a   : gr.o regression.o $(util_gr_objs)
	ar cr $@ $^

libers.a   : er.o regression.o $(util_er_objs)
	ar cr $@ $^

libldas.a  : lda.o             $(util_lda_objs)
	ar cr $@ $^                
                               
libaldas.a : author_lda.o      $(util_lda_objs)
	ar cr $@ $^                
                               
libjldas.a : joint_lda.o       $(util_lda_objs)
	ar cr $@ $^                
                               
libbldas.a : bri_lda.o         $(util_lda_objs)
	ar cr $@ $^                
                               
libtots.a  : tot.o             $(util_lda_objs)
	ar cr $@ $^                
                               
libtotgs.a : totg.o            $(util_lda_objs)
	ar cr $@ $^

libpmfs.a   : pmf.o  mf.o      $(util_pmf_objs)
	ar cr $@ $^

libbmfs.a   : bmf.o  mf.o      $(util_pmf_objs)
	ar cr $@ $^


#-----------------------------------------------------------------------
# Clean up
#-----------------------------------------------------------------------

.PHONY : clean

clean:
	rm -f $(PRGS) *.o *.so *.a


