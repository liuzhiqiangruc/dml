objs    = kmeans.o       k2d.o          louvain.o
sos     = libkmeans.so   libk2d.so      liblouvain.so
statics = libkmeanss.a   libk2ds.a      liblouvains.a

all : $(objs) $(statics) $(sos)

lib_install :
	mkdir -p ../lib
	cp *.a ../lib
	cp *.so ../lib

inc_install :
	mkdir -p ../inc
	cp *.h ../inc

libk2d.so: k2d.o
	$(LD) $^ -o $@

libk2ds.a: k2d.o
	ar cr $@ $^

libkmeans.so : kmeans.o
	$(LD) $^ -o $@

libkmeanss.a : kmeans.o
	ar cr $@ $^


liblouvain.so : louvain.o ../util/hash.o ../util/str.o
	$(LD) $^ -o $@

liblouvains.a : louvain.o ../util/hash.o ../util/str.o
	ar cr $@ $^


install :
	$(MAKE) lib_install
	$(MAKE) inc_install


.PHONY: clean
clean :
	rm -rf $(sos)
	rm -rf $(objs)
	rm -rf $(statics)
