CC = gcc -std=c99 -g
CFBASE = -pedantic -pipe -Wall
CFLAGS = $(CFBASE) -O3 -fpic -DNDEBUG -DNPRINT -I../util
LD = gcc -shared



objs    = lda.o        lda_xy.o        hlda.o
sos     = liblda.so    liblda_xy.so    libhlda.so
statics = libldas.a    liblda_xys.a    libhldas.a

all : $(objs) $(statics) $(sos)

lib_install :
	mkdir -p ../lib
	cp *.a ../lib
	cp *.so ../lib

inc_install :
	mkdir -p ../inc
	cp *.h ../inc


lda.o     : lda.h
lda_xy.o  : lda.h
hlda.o    : lda.h


liblda.so : lda.o ../util/str.o ../util/hash.o
	$(LD) $^ -o $@

liblda_xy.so : lda_xy.o ../util/str.o ../util/hash.o
	$(LD) $^ -o $@

libhlda.so : hlda.o ../util/str.o ../util/hash.o
	$(LD) $^ -o $@

libldas.a : lda.o ../util/str.o ../util/hash.o
	ar cr $@ $^

liblda_xys.a : lda_xy.o ../util/str.o ../util/hash.o
	ar cr $@ $^

libhldas.a : hlda.o ../util/str.o ../util/hash.o
	ar cr $@ $^

install :
	$(MAKE) lib_install
	$(MAKE) inc_install


.PHONY: clean
clean :
	rm -rf $(sos)
	rm -rf $(objs)
	rm -rf $(statics)
