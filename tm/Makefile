CC = gcc -std=c99 -g
CFBASE = -pedantic -pipe -Wall
CFLAGS = $(CFBASE) -O0 -fpic -DNDEBUG -DNPRINT -I../util
LD = gcc -shared



objs    = lda.o
sos     = liblda.so
statics = libldas.a

all : $(objs) $(statics) $(sos)

lib_install :
	mkdir -p ../lib
	cp *.a ../lib
	cp *.so ../lib

inc_install :
	mkdir -p ../inc
	cp *.h ../inc


lda.o     : lda.h


liblda.so : lda.o ../util/str.o ../util/hash.o
	$(LD) $^ -o $@

libldas.a : lda.o ../util/str.o ../util/hash.o
	ar cr $@ $^

install :
	$(MAKE) lib_install
	$(MAKE) inc_install


.PHONY: clean
clean :
	rm -rf $(sos)
	rm -rf $(objs)
	rm -rf $(statics)
