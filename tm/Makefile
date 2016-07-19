CC = gcc -std=c99 -g
CFBASE = -pedantic -pipe -Wall
CFLAGS = $(CFBASE) -O3 -fpic -DNDEBUG -DNPRINT -I../util
LD = gcc -shared



objs    = lda.o        lda_xy.o        hlda.o       tm_config.o   tm_config_h.o  tm.o htm.o
sos     = liblda.so    liblda_xy.so    libhlda.so   libtm.so  libhtm.so
statics = libldas.a    liblda_xys.a    libhldas.a   libtms.a  libhtms.a

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
tm_config.o : tm_config.h
tm_config_h.o : tm_config_h.h
tm.o      : tm.h
htm.o     : tm.h


libtm.so  : tm.o tm_config.o ../util/str.o ../util/hash.o
	$(LD) $^ -o $@

libhtm.so : htm.o tm_config.o tm_config_h.o ../util/str.o ../util/hash.o
	$(LD) $^ -o $@

liblda.so : lda.o ../util/str.o ../util/hash.o
	$(LD) $^ -o $@

liblda_xy.so : lda_xy.o ../util/str.o ../util/hash.o
	$(LD) $^ -o $@

libhlda.so : hlda.o ../util/str.o ../util/hash.o
	$(LD) $^ -o $@


libtms.a  : tm.o tm_config.o ../util/str.o ../util/hash.o
	ar cr $@ $^

libhtms.a : htm.o tm_config.o tm_config_h.o ../util/str.o ../util/hash.o
	ar cr $@ $^

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
