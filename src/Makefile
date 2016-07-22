INC = ../inc
LIB = ../lib

INS = ../output


CC = gcc -std=c99 -g
CFBASE = -Wall -pedantic -pipe
CFLAGS = $(CFBASE) -DNDEBUG -DNPRINT -O3 -I$(INC)

PRGS   = lr gblr k2d lda hlda xlda

all : $(PRGS)

tm.o : tm.c
	$(CC) $(CFLAGS) -c $^ -o $@

lr.o : lr.c
	$(CC) $(CFLAGS) -c $^ -o $@

gblr.o : gblr.c
	$(CC) $(CFLAGS) -c $^ -o $@

k2d.o : k2d.c
	$(CC) $(CFLAGS) -c $^ -o $@

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
	$(CC) -L$(LIB) $^ -lgblrs -lm -o $@

k2d : k2d.o
	$(CC) -L$(LIB) $^ -lk2ds -lm -o $@

bin_install:
	cp $(PRGS)  $(INS)/bin

install:
	-mkdir -p $(INS)/bin
	$(MAKE) bin_install

.PHONY: clean
clean :
	rm -rf $(PRGS)
	rm -rf *.o
