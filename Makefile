MAKE     = make
UTILDIR  = util
REGRDIR  = regr
GBDTDIR  = gbdt
TMDIR    = tm
TSDIR    = ts
CLSDIR   = cls
NNDIR    = nn
REPDIR   = repo
SRC      = main
INSTDIR  = .


INC = -I../$(UTILDIR) -I../$(REPDIR) -I../$(REGRDIR)

CC = gcc -std=c99 -O3
LD = gcc -shared
WARN = -Wall -Wno-unused-but-set-variable
CFBASE = $(WARN) -pedantic -pipe
CFLAGS = $(CFBASE) -fPIC -DNDEBUG -DNPRINT -DDTREE_DEBUG $(INC)

export CC LD CFLAGS MAKE

all:

love: libs app clean

libs:
	cd $(UTILDIR); $(MAKE) clean; $(MAKE); $(MAKE) install
	cd $(REGRDIR); $(MAKE) clean; $(MAKE); $(MAKE) install
	cd $(GBDTDIR); $(MAKE) clean; $(MAKE); $(MAKE) install
	cd $(TMDIR);   $(MAKE) clean; $(MAKE); $(MAKE) install
	cd $(TSDIR);   $(MAKE) clean; $(MAKE); $(MAKE) install
	cd $(CLSDIR);  $(MAKE) clean; $(MAKE); $(MAKE) install
	cd $(NNDIR);   $(MAKE) clean; $(MAKE); $(MAKE) install

app :
	cd $(SRC); $(MAKE) clean ; $(MAKE); $(MAKE) install
	make lib_install


# --------------
# install
# --------------
lib_install:
	cp -r lib output
	cp -r inc output

#-----------------------------------------------------------------------
# Clean up
#-----------------------------------------------------------------------

.PHONY : localclean clean

localclean:
	rm -rf inc
	rm -rf lib

clean:
	$(MAKE) localclean
	cd $(UTILDIR);  $(MAKE) clean
	cd $(REGRDIR);  $(MAKE) clean
	cd $(GBDTDIR);  $(MAKE) clean
	cd $(SRC);      $(MAKE) clean
	cd $(TMDIR);    $(MAKE) clean
	cd $(TSDIR);    $(MAKE) clean
	cd $(CLSDIR);   $(MAKE) clean
	cd $(NNDIR);    $(MAKE) clean
	cd $(REPDIR);   $(MAKE) clean
