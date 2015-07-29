CC       = gcc -std=c99
CFBASE   = -Wall -pedantic -pipe 
CFLAGS   = $(CFBASE) -DNDEBUG -DNPRINT -O3 -fpic

LD       = gcc
SOFLAGS  = -shared 

OBJS     = hashtable.o heap.o   fptree.o  str.o      md5.o     mtrace.o  queue.o    rbtree.o   set.o      idmap.o      
SOS      = libht.so    libhp.so libfpt.so libstr.so  libmd5.so libque.so librbt.so  libmed.so  libset.so  libidmap.so  
STATICS  = libhts.a    libhps.a libfpts.a libstrs.a  libmd5s.a libques.a librbts.a  libmeds.a  libsets.a  libidmaps.a  

all:    $(STATICS)
so :    $(SOS)

## --------- objects --------------

str.o:         str.h
hashtable.o:   hashtable.h str.h
heap.o:        heap.h 
fptree.o:      fptree.h str.h hashtable.h
md5.o:         md5.h 
queue.o:       queue.h 
rbtree.o:      rbtree.h fn_type.h
mtrace.o:      mtrace.h fn_type.h rbtree.h
set.o:         set.h rbtree.h fn_type.h
idmap.o:       idmap.h set.h rbtree.h fn_type.h


## --------- .so --------------

libstr.so : str.o
	$(LD) $(SOFLAGS) $^ -o $@

libht.so  : hashtable.o str.o
	$(LD) $(SOFLAGS) $^ -o $@

libque.so : queue.o
	$(LD) $(SOFLAGS) $^ -o $@

libhp.so  : heap.o 
	$(LD) $(SOFLAGS) $^ -o $@

libfpt.so : fptree.o hashtable.o str.o
	$(LD) $(SOFLAGS) $^ -o $@

libmd5.so : md5.o
	$(LD) $(SOFLAGS) $^ -o $@

librbt.so : rbtree.o 
	$(LD) $(SOFLAGS) $^ -o $@

libmed.so : mtrace.o rbtree.o
	$(LD) $(SOFLAGS) $^ -o $@

libset.so : set.o rbtree.o
	$(LD) $(SOFLAGS) $^ -o $@

libidmap.so : idmap.o set.o rbtree.o
	$(LD) $(SOFLAGS) $^ -o $@


## --------- .a --------------

libstrs.a:str.o
	ar cr $@ $^

libhts.a: hashtable.o str.o
	ar cr $@ $^

libhps.a: heap.o
	ar cr $@ $^

libques.a : queue.o
	ar cr $@ $^

libfpts.a : fptree.o hashtable.o str.o
	ar cr $@ $^

libmd5s.a : md5.o
	ar cr $@ $^

librbts.a : rbtree.o
	ar cr $@ $^

libmeds.a : mtrace.o rbtree.o
	ar cr $@ $^

libsets.a : set.o rbtree.o
	ar cr $@ $^

libidmaps.a : set.o idmap.o rbtree.o
	ar cr $@ $^

#-----------------------------------------------------------------------
# Clean up
#-----------------------------------------------------------------------

.PHONY: clean

clean:
	rm -f $(PRGS) *.o *.so *.a
