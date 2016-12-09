objs    =  tm_config.o    tm_config_h.o  tm_config_x.o tm.o htm.o xtm.o 
sos     =  libtm.so  libhtm.so  libxtm.so
statics =  libtms.a  libhtms.a  libxtms.a 

all : $(objs) $(statics) $(sos)

lib_install :
	mkdir -p ../lib
	cp *.a ../lib
	cp *.so ../lib

inc_install :
	mkdir -p ../inc
	cp *.h ../inc


tm_config.o : tm_config.h
tm_config_h.o : tm_config_h.h
tm_config_x.o : tm_config_x.h
tm_config_d.o : tm_config_d.h
tm.o      : tm.h
htm.o     : tm.h
xtm.o     : tm.h

libtm.so  : tm.o tm_config.o ../util/str.o ../util/hash.o
	$(LD) $^ -o $@

libhtm.so : htm.o tm_config.o tm_config_h.o ../util/str.o ../util/hash.o
	$(LD) $^ -o $@

libxtm.so : xtm.o tm_config.o tm_config_h.o tm_config_x.o ../util/str.o ../util/hash.o
	$(LD) $^ -o $@


libtms.a  : tm.o tm_config.o ../util/str.o ../util/hash.o
	ar cr $@ $^

libhtms.a : htm.o tm_config.o tm_config_h.o ../util/str.o ../util/hash.o
	ar cr $@ $^

libxtms.a : xtm.o tm_config.o tm_config_h.o tm_config_x.o ../util/str.o ../util/hash.o
	ar cr $@ $^


install :
	$(MAKE) lib_install
	$(MAKE) inc_install


.PHONY: clean
clean :
	rm -rf $(sos)
	rm -rf $(objs)
	rm -rf $(statics)
