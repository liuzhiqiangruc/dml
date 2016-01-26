INC = ../inc
LIB = ../lib

INS = ../output


CC = gcc -std=c99 -g
CFBASE = -Wall -pedantic -pipe
CFLAGS = $(CFBASE) -DNDEBUG -DNPRINT -O0 -I$(INC)

PRGS   = lr lda

all : $(PRGS)

lr.o : lr.c
	$(CC) $(CFLAGS) -c $^ -o $@

lda.o : lda.c
	$(CC) $(CFLAGS) -c $^ -o $@

lr : lr.o
	$(CC) -L$(LIB) $^ -llrs -lm -o $@

lda: lda.o
	$(CC) -L$(LIB) $^ -lldas -lm -o $@

bin_install:
	cp lr $(INS)/bin
	cp lda $(INS)/bin

install:
	-mkdir -p $(INS)/bin
	$(MAKE) bin_install

.PHONY: clean
clean :
	rm -rf $(PRGS)
	rm -rf *.o
