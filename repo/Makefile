
CC = gcc -std=c99 -g
CFBASE = -pedantic -pipe -Wall
CFLAGS = $(CFBASE) -O0 -fpic -DNDEBUG -DNPRINT -I../util
LD = gcc -shared

objs    =  repo.o

all : $(objs)

repo.o  : repo.h

.PHONY: clean
clean :
	rm -rf $(objs)
