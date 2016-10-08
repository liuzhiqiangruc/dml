CC = gcc -std=c99 -g
CFBASE = -pedantic -pipe -Wall
CFLAGS = $(CFBASE) -O0 -fpic -DNDEBUG -DNPRINT -I../util -I../repo
LD = gcc -shared

objs    =  w2v.o w2v_config.o
sos     =  libw2v.so        
statics =  libw2vs.a 

all : $(objs) $(statics) $(sos)

lib_install :
	mkdir -p ../lib
	cp *.a ../lib
	cp *.so ../lib

inc_install :
	mkdir -p ../inc
	cp *.h ../inc


w2v.o  : w2v.h
w2v_config.o : w2v_config.h

libw2v.so  : w2v.o ../repo/repo.o w2v_config.o ../util/str.o ../util/hash.o
	$(LD) $^ -o $@

libw2vs.a  : w2v.o ../repo/repo.o w2v_config.o ../util/str.o ../util/hash.o
	ar cr $@ $^

install :
	$(MAKE) lib_install
	$(MAKE) inc_install


.PHONY: clean
clean :
	rm -rf $(sos)
	rm -rf $(objs)
	rm -rf $(statics)
