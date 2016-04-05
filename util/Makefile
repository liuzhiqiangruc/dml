CC = gcc -std=c99 -g
CFBASE = -pedantic -pipe -Wall
CFLAGS = $(CFBASE) -O3 -fpic -DNDEBUG -DNPRINT
LD = gcc -shared



objs    = hash.o     rbtree.o    str.o       median.o
sos     = libhs.so   librbt.so   libstr.so   libmed.so
statics = libhss.a   librbts.a   libstrs.a   libmeds.a

all : $(objs) $(statics) $(sos)

lib_install :
	mkdir -p ../lib
	cp *.a ../lib
	cp *.so ../lib

inc_install :
	mkdir -p ../inc
	cp *.h ../inc


hash.o : hash.h
rbtree.o  : rbtree.h
str.o    : str.h

libhs.so : hash.o
	$(LD) $^ -o $@
librbt.so : rbtree.o
	$(LD) $^ -o $@
libstr.so : str.o
	$(LD) $^ -o $@
libmed.so : median.o rbtree.o
	$(LD) $^ -o $@

libhss.a : hash.o
	ar cr $@ $^
librbts.a : rbtree.o
	ar cr $@ $^
libstrs.a : str.o
	ar cr $@ $^
libmeds.a : median.o rbtree.o
	ar cr $@ $^


install :
	$(MAKE) lib_install
	$(MAKE) inc_install


.PHONY: clean
clean :
	rm -rf $(sos)
	rm -rf $(objs)
	rm -rf $(statics)
