CC = gcc -std=c99 -g
CFBASE = -pedantic -pipe -Wall
CFLAGS = $(CFBASE) -O3 -fpic -DNDEBUG -DNPRINT -I../util
LD = gcc -shared



objs    =  lda_xy.o       tm_config.o tm_config_h.o  tm_config_x.o tm.o htm.o xtm.o
sos     =  liblda_xy.so   libtm.so  libhtm.so  libxtm.so
statics =  liblda_xys.a   libtms.a  libhtms.a  libxtms.a

all : $(objs) $(statics) $(sos)

lib_install :
	mkdir -p ../lib
	cp *.a ../lib
	cp *.so ../lib

inc_install :
	mkdir -p ../inc
	cp *.h ../inc


lda_xy.o  : lda.h
tm_config.o : tm_config.h
tm_config_h.o : tm_config_h.h
tm_config_x.o : tm_config_x.h
tm.o      : tm.h
htm.o     : tm.h
xtm.o     : tm.h

libtm.so  : tm.o tm_config.o ../util/str.o ../util/hash.o
	$(LD) $^ -o $@

libhtm.so : htm.o tm_config.o tm_config_h.o ../util/str.o ../util/hash.o
	$(LD) $^ -o $@

libxtm.so : xtm.o tm_config.o tm_config_h.o tm_config_x.o ../util/str.o ../util/hash.o
	$(LD) $^ -o $@

liblda_xy.so : lda_xy.o ../util/str.o ../util/hash.o
	$(LD) $^ -o $@

libtms.a  : tm.o tm_config.o ../util/str.o ../util/hash.o
	ar cr $@ $^

libhtms.a : htm.o tm_config.o tm_config_h.o ../util/str.o ../util/hash.o
	ar cr $@ $^

libxtms.a : xtm.o tm_config.o tm_config_h.o tm_config_x.o ../util/str.o ../util/hash.o
	ar cr $@ $^


liblda_xys.a : lda_xy.o ../util/str.o ../util/hash.o
	ar cr $@ $^

install :
	$(MAKE) lib_install
	$(MAKE) inc_install


.PHONY: clean
clean :
	rm -rf $(sos)
	rm -rf $(objs)
	rm -rf $(statics)
