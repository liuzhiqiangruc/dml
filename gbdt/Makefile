CC = gcc -std=c99 -g
CFBASE = -pedantic -pipe -Wall
CFLAGS = $(CFBASE) -O3 -fpic -DNDEBUG -DNPRINT -I../util -I../regr
LD = gcc -shared

ext_obj = ../util/hash.o ../util/str.o ../regr/auc.o

objs    = gblr.o gbdt.o dtree.o tdata.o
sos     = libgblr.so libgbdt.so libdtree.so libtdata.so
statics = libgblrs.a libgbdts.a libdtrees.a libtdatas.a

all : $(objs) $(statics) $(sos)

lib_install :
	mkdir -p ../lib
	cp *.a ../lib
	cp *.so ../lib

inc_install :
	mkdir -p ../inc
	cp *.h ../inc

gblr.o : gblr.h
gbdt.o : gbdt.h
dtree.o : dtree.h
tdata.o : tdata.h

libgblr.so : gblr.o gbdt.o dtree.o tdata.o $(ext_obj)
	$(LD) $^ -o $@

libgbdt.so : gbdt.o dtree.o tdata.o $(ext_obj)
	$(LD) $^ -o $@

libdtree.so : dtree.o tdata.o $(ext_obj) 
	$(LD) $^ -o $@

libtdata.so : tdata.o $(ext_obj)
	$(LD) $^ -o $@


libgblrs.a : gblr.o gbdt.o dtree.o tdata.o $(ext_obj)
	ar cr $@ $^

libgbdts.a : gbdt.o dtree.o tdata.o $(ext_obj) 
	ar cr $@ $^

libdtrees.a : dtree.o tdata.o $(ext_obj)
	ar cr $@ $^

libtdatas.a : tdata.o  $(ext_obj)
	ar cr $@ $^


install :
	$(MAKE) lib_install
	$(MAKE) inc_install


.PHONY: clean
clean :
	rm -rf $(sos)
	rm -rf $(objs)
	rm -rf $(statics)
