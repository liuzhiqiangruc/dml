objs    =  tsdata.o w2v_config.o d2v_config.o rnnlm.o rnn_config.o hsoft.o w2v.o d2v.o
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
w2v.o : w2v.h
d2v.o : d2v.h
w2v_config.o : w2v_config.h
d2v_config.o : d2v_config.h
rnn_config.o : rnn_config.h


libw2v.so : w2v.o ../repo/repo.o w2v_config.o ../util/str.o ../util/hash.o tsdata.o hsoft.o
	$(LD) $^ -o $@

libw2vs.a : w2v.o ../repo/repo.o w2v_config.o ../util/str.o ../util/hash.o tsdata.o hsoft.o
	ar cr $@ $^

libd2v.so : d2v.o ../repo/repo.o d2v_config.o w2v_config.o ../util/str.o ../util/hash.o tsdata.o hsoft.o
	$(LD) $^ -o $@

libd2vs.a : d2v.o ../repo/repo.o d2v_config.o w2v_config.o ../util/str.o ../util/hash.o tsdata.o hsoft.o
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
