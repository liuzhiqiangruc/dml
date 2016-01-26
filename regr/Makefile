CC = gcc -std=c99 -g
CFBASE = -pedantic -pipe -Wall
CFLAGS = $(CFBASE) -O0 -fpic -DNDEBUG -DNPRINT -I../util
LD = gcc -shared



objs    = auc.o lr.o regress.o newton.o
sos     = libauc.so liblr.so libnewton.so
statics = libaucs.a liblrs.a libnewtons.a

all : $(objs) $(statics) $(sos)

lib_install :
	mkdir -p ../lib
	cp *.a ../lib
	cp *.so ../lib

inc_install :
	mkdir -p ../inc
	cp *.h ../inc


auc.o     : auc.h
newton.o  : newton.h
regress.o : regress.h
lr.o      : lr.h

libauc.so : auc.o
	$(LD) $^ -o $@
libnewton.so : newton.o
	$(LD) $^ -o $@
liblr.so  : lr.o regress.o newton.o auc.o ../util/hash.o ../util/str.o
	$(LD) $^ -o $@


libaucs.a : auc.o
	ar cr $@ $^
libnewtons.a : newton.o
	ar cr $@ $^
liblrs.a  : lr.o regress.o newton.o auc.o ../util/hash.o ../util/str.o
	ar cr $@ $^


install :
	$(MAKE) lib_install
	$(MAKE) inc_install


.PHONY: clean
clean :
	rm -rf $(sos)
	rm -rf $(objs)
	rm -rf $(statics)
