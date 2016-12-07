CC = gcc -std=c99 -g
CFBASE = -pedantic -pipe -Wall
CFLAGS = $(CFBASE) -O0 -fpic -DNDEBUG -DNPRINT -I../util -I../repo
LD = gcc -shared

objs    =  tsdata.o w2v_config.o d2v_config.o vec.o word2vec.o doc2vec.o  rnnlm.o rnn_config.o hsoft.o
sos     =  libw2v.so   libd2v.so   librnnlm.so
statics =  libw2vs.a   libd2vs.a   librnnlms.a

all : $(objs) $(statics) $(sos)

lib_install :
	mkdir -p ../lib
	cp *.a ../lib
	cp *.so ../lib

inc_install :
	mkdir -p ../inc
	cp *.h ../inc

tsdata.o : tsdata.h
vec.o : vec.h
hsoft.o : hsoft.h
rnnlm.o   : rnnlm.h
word2vec.o  : word2vec.h
doc2vec.o  : doc2vec.h
w2v_config.o : w2v_config.h
d2v_config.o : d2v_config.h
rnn_config.o : rnn_config.h

libw2v.so  : word2vec.o ../repo/repo.o w2v_config.o ../util/str.o ../util/hash.o tsdata.o vec.o
	$(LD) $^ -o $@

libw2vs.a  : word2vec.o ../repo/repo.o w2v_config.o ../util/str.o ../util/hash.o tsdata.o vec.o
	ar cr $@ $^


libd2v.so  : doc2vec.o ../repo/repo.o w2v_config.o d2v_config.o ../util/str.o ../util/hash.o tsdata.o vec.o
	$(LD) $^ -o $@

libd2vs.a  : doc2vec.o ../repo/repo.o w2v_config.o d2v_config.o ../util/str.o ../util/hash.o tsdata.o vec.o
	ar cr $@ $^

librnnlm.so : rnnlm.o rnn_config.o hsoft.o tsdata.o ../util/str.o ../util/hash.o
	$(LD) $^ -o $@

librnnlms.a : rnnlm.o rnn_config.o hsoft.o tsdata.o ../util/str.o ../util/hash.o
	ar cr $@ $^


install :
	$(MAKE) lib_install
	$(MAKE) inc_install


.PHONY: clean
clean :
	rm -rf $(sos)
	rm -rf $(objs)
	rm -rf $(statics)
