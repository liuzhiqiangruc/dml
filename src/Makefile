INC = ../inc
LIB = ../lib

INS = ../output


CC = gcc -std=c99 -g
CFBASE = -Wall -pedantic -pipe
CFLAGS = $(CFBASE) -DNDEBUG -DNPRINT -O0 -I$(INC)

PRGS   = lr lda gblr lda_xy k2d

all : $(PRGS)

lr.o : lr.c
	$(CC) $(CFLAGS) -c $^ -o $@

lda.o : lda.c
	$(CC) $(CFLAGS) -c $^ -o $@

gblr.o : gblr.c
	$(CC) $(CFLAGS) -c $^ -o $@

k2d.o : k2d.c
	$(CC) $(CFLAGS) -c $^ -o $@

lr : lr.o
	$(CC) -L$(LIB) $^ -llrs -lm -o $@

lda: lda.o
	$(CC) -L$(LIB) $^ -lldas -lm -o $@

lda_xy: lda.o
	$(CC) -L$(LIB) $^ -llda_xys -lm -o $@

gblr: gblr.o
	$(CC) -L$(LIB) $^ -lgblrs -lm -o $@

k2d : k2d.o
	$(CC) -L$(LIB) $^ -lk2ds -lm -o $@

bin_install:
	cp lr $(INS)/bin
	cp lda $(INS)/bin
	cp lda_xy $(INS)/bin
	cp gblr $(INS)/bin
	cp k2d $(INS)/bin

install:
	-mkdir -p $(INS)/bin
	$(MAKE) bin_install

.PHONY: clean
clean :
	rm -rf $(PRGS)
	rm -rf *.o
