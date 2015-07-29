DM       = ../dm
UTIL     = ../util
INCS     = -I../util -I../dm
LIBS     = -L../util -L../dm

CC       = gcc -std=c99
CFBASE   = -Wall -pedantic -pipe
CFLAGS   = $(CFBASE) -DNDEBUG -DNPRINT -O3 -fpic 

LD       = gcc
SOFLAGS  = -shared

OBJS     = dp.o     viterbi.o  edm.o     hash_pattern.o loess.o      fit_newtom.o
SOS      = libdp.so libvtb.so  libedm.so libhpat.so     libloess.so  libfitnt.so
STATICS  = libdps.a libvtbs.a  libedms.a libhpats.a     libloesss.a  libfitnts.a

util_mtrace_objs = $(UTIL)/mtrace.o $(UTIL)/rbtree.o
fit_newton_objs  = $(DM)/newton.o 


#-----------------------------------------------------------------------
# targets
#-----------------------------------------------------------------------

all:	$(STATICS)
so:     $(SOS)
objs:   $(OBJS)

#-----------------------------------------------------------------------
# objs
#-----------------------------------------------------------------------

dp.o           : dp.h
viterbi.o      : viterbi.h
hash_pattern.o : hash_pattern.h

edm.o          : edm.c edm.h
	$(CC) $(CFLAGS) $(INCS) -c $< -o $@

loess.o        : loess.c loess.h
	$(CC) $(CFLAGS) $(INCS) -c $< -o $@

fit_newton.o   : fit_newton.c fit_newton.h
	$(CC) $(CFLAGS) $(INCS) -c $< -o $@

#-----------------------------------------------------------------------
# shared libs
#-----------------------------------------------------------------------

libdp.so  : dp.o
	$(LD) $(SOFLAGS) $^ -o $@

libvtb.so : viterbi.o
	$(LD) $(SOFLAGS) $^ -o $@

libhpat.so : hash_pattern.o
	$(LD) $(SOFLAGS) $^ -o $@

libedm.so : edm.o                  $(util_mtrace_objs)
	$(LD) $(SOFLAGS) $^ -o $@

libloess.so : loess.o              $(util_mtrace_objs)
	$(LD) $(SOFLAGS) $^ -o $@

libfitnt.so : fit_newton.o         $(fit_newton_objs)
	$(LD) $(SOFLAGS) $^ -o $@


#-----------------------------------------------------------------------
# statics
#-----------------------------------------------------------------------
libdps.a : dp.o
	ar cr $@ $^

libvtbs.a : viterbi.o
	ar cr $@ $^

libhpats.a : hash_pattern.o
	ar cr $@ $^

libedms.a : edm.o                  $(util_mtrace_objs)
	ar cr $@ $^                    
                                   
libloesss.a : loess.o              $(util_mtrace_objs)
	ar cr $@ $^                    
                                   
libfitnts.a : fit_newton.o         $(fit_newton_objs)
	ar cr $@ $^

#-----------------------------------------------------------------------
# Clean up
#-----------------------------------------------------------------------
.PHONY : clean

clean:
	rm -f $(PRGS) *.o *.so *.a

