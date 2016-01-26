CC       = gcc -std=c99
CFBASE   = -Wall -pedantic -pipe
CFLAGS   = $(CFBASE) -DNDEBUG -DNPRINT -O0 -fpic -I../util

UTIL     = ../util
INCS     = -I../util
LIBS     = -L../util


LD       = gcc -shared

objs =      breakout.o dist.o    loess.o     modl.o     pattern.o viterbi.o
sos =       libbrk.so  libdst.so libloess.so libmodl.so libpat.so libvtb.so
statics =   libbrks.a  libdsts.a libloesss.a libmodls.a libpats.a libvtbs.a

all : $(objs) $(statics) $(sos)

lib_install:
	mkdir -p ../lib
	cp *.a ../lib
	cp *.so ../lib

inc_install:
	mkdir -p ../inc
	cp *.h ../inc

install:
	make lib_install
	make inc_install

libbrk.so : breakout.o ../util/median.o ../util/rbtree.o
	$(LD) $(SOFLAGS) $^ -o $@

libloess.so : loess.o  ../util/median.o ../util/rbtree.o
	$(LD) $(SOFLAGS) $^ -o $@

libvtb.so : viterbi.o
	$(LD) $(SOFLAGS) $^ -o $@

libpat.so : pattern.o
	$(LD) $(SOFLAGS) $^ -o $@

libdst.so : dist.o
	$(LD) $(SOFLAGS) $^ -o $@

libmodl.so : modl.o
	$(LD) $(SOFLAGS) $^ -o $@


libvtbs.a : viterbi.o
	ar cr $@ $^

libpats.a : pattern.o
	ar cr $@ $^

libmodls.a : modl.o
	ar cr $@ $^

libdsts.a : dist.o
	ar cr $@ $^                    

libbrks.a : breakout.o  ../util/median.o ../util/rbtree.o
	ar cr $@ $^                    
                                   
libloesss.a : loess.o   ../util/median.o ../util/rbtree.o
	ar cr $@ $^                    
                                   

#-----------------------------------------------------------------------
# Clean up
#-----------------------------------------------------------------------
.PHONY : clean

clean:
	rm -rf $(sos)
	rm -rf $(objs)
	rm -rf $(statics)

