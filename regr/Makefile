objs    = auc.o lr.o regr.o regcfg.o deeplr.o
sos     = libauc.so liblr.so  libdeeplr.so
statics = libaucs.a liblrs.a  libdeeplrs.a

all : $(objs) $(statics) $(sos)

lib_install :
	mkdir -p ../lib
	cp *.a ../lib
	cp *.so ../lib

inc_install :
	mkdir -p ../inc
	cp *.h ../inc


auc.o     : auc.h
regr.o    : regr.h
regcfg.o  : regcfg.h
deeplr.o  : deeplr.h
lr.o      : lr.h

libauc.so : auc.o
	$(LD) $^ -o $@

liblr.so  : lr.o regr.o auc.o regcfg.o ../util/hash.o ../util/str.o ../util/data.o
	$(LD) $^ -o $@

libdeeplr.so  : deeplr.o regr.o auc.o regcfg.o ../util/hash.o ../util/str.o ../util/data.o
	$(LD) $^ -o $@


libaucs.a : auc.o
	ar cr $@ $^

liblrs.a  : lr.o regr.o auc.o regcfg.o ../util/hash.o ../util/str.o ../util/data.o
	ar cr $@ $^

libdeeplrs.a : deeplr.o regr.o auc.o regcfg.o ../util/hash.o ../util/str.o ../util/data.o
	ar cr $@ $^

install :
	$(MAKE) lib_install
	$(MAKE) inc_install


.PHONY: clean
clean :
	rm -rf $(sos)
	rm -rf $(objs)
	rm -rf $(statics)
