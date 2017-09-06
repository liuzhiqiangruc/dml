INC = ../inc
LIB = ../lib
INS = ../output


CFLAGS = $(CFBASE) -DNDEBUG -DNPRINT -I$(INC)

PRGS   = lr gblr k2d lda hlda xlda w2v d2v kmeans gbm gbrank

all : $(PRGS)

w2v.o : w2v.c
	$(CC) $(CFLAGS) -c $^ -o $@

d2v.o : d2v.c
	$(CC) $(CFLAGS) -c $^ -o $@

wpred.o : wpred.c
	$(CC) $(CFLAGS) -c $^ -o $@

tm.o : tm.c
	$(CC) $(CFLAGS) -c $^ -o $@

lr.o : lr.c
	$(CC) $(CFLAGS) -c $^ -o $@

gblr.o : gblr.c
	$(CC) $(CFLAGS) -c $^ -o $@

gbrank.o : gbrank.c
	$(CC) $(CFLAGS) -c $^ -o $@

gbm.o  : gbm.c
	$(CC) $(CFLAGS) -c $^ -o $@

k2d.o : k2d.c
	$(CC) $(CFLAGS) -c $^ -o $@

kmeans.o : kmeans.c
	$(CC) $(CFLAGS) -c $^ -o $@

w2v : w2v.o
	$(CC) -L$(LIB) $^ -lw2vs -lm -o $@

d2v : d2v.o
	$(CC) -L$(LIB) $^ -ld2vs -lm -o $@

wpred : wpred.o
	$(CC) -L$(LIB) $^ -lw2vs -lm -o $@

lda : tm.o
	$(CC) -L$(LIB) $^ -ltms -lm -o $@

hlda : tm.o
	$(CC) -L$(LIB) $^ -lhtms -lm -o $@

xlda : tm.o
	$(CC) -L$(LIB) $^ -lxtms -lm -o $@

lr : lr.o
	$(CC) -L$(LIB) $^ -llrs -lm -o $@

lda_xy: lda.o
	$(CC) -L$(LIB) $^ -llda_xys -lm -o $@

gblr: gblr.o
	$(CC) -L$(LIB) $^ -lgblrs -lm  -lpthread -o $@

gbrank: gbrank.o
	$(CC) -L$(LIB) $^ -lgbranks -lm  -lpthread -o $@

gbm : gbm.o
	$(CC) -L$(LIB) $^ -lgbmrs -lm  -lpthread -o $@

k2d : k2d.o
	$(CC) -L$(LIB) $^ -lk2ds -lm -o $@

kmeans : kmeans.o
	$(CC) -L$(LIB) $^ -lkmeanss -lm -o $@

bin_install:
	cp $(PRGS)  $(INS)/bin

install:
	-mkdir -p $(INS)/bin
	$(MAKE) bin_install

.PHONY: clean
clean :
	rm -rf $(PRGS)
	rm -rf *.o
	rm -rf *.dSYM
