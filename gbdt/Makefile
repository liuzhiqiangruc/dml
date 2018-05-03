ext_obj = ../util/hash.o ../util/str.o ../regr/auc.o ../util/data.o

objs    = dtree.o    gbm.o gbmr.o 
sos     = libdtree.so   libgbm.so libgbmr.so 
statics = libdtrees.a   libgbms.a libgbmrs.a 

all : $(objs) $(statics) $(sos)

lib_install :
	mkdir -p ../lib
	cp *.a ../lib
	cp *.so ../lib

inc_install :
	mkdir -p ../inc
	cp *.h ../inc

gbmr.o : gbmr.h
gbm.o  : gbm.h
dtree.o : dtree.h

libgbmr.so : gbmr.o gbm.o dtree.o $(ext_obj)
	$(LD) $^ -o $@

libgbm.so : gbm.o dtree.o $(ext_obj)
	$(LD) $^ -o $@

libdtree.so : dtree.o $(ext_obj) 
	$(LD) $^ -o $@



libgbmrs.a : gbmr.o gbm.o dtree.o $(ext_obj)
	ar cr $@ $^

libgbms.a : gbm.o dtree.o $(ext_obj) 
	ar cr $@ $^

libdtrees.a : dtree.o  $(ext_obj)
	ar cr $@ $^


install :
	$(MAKE) lib_install
	$(MAKE) inc_install


.PHONY: clean
clean :
	rm -rf $(sos)
	rm -rf $(objs)
	rm -rf $(statics)
