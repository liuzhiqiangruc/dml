UTILDIR  = util
TS       = ts
DM       = dm
SRC      = src
INSTDIR  = output



all: lib_statics app so

lib_statics:
	cd $(UTILDIR);  $(MAKE)
	cd $(DM);       $(MAKE)
	cd $(TS);       $(MAKE)

app :
	cd $(SRC);      $(MAKE) clean ; $(MAKE)

so  :
	cd $(UTILDIR);  $(MAKE) so
	cd $(TS);       $(MAKE) so 
	cd $(DM);       $(MAKE) so

#-----------------------------------------------------------------------
# Installation
#-----------------------------------------------------------------------
APP_INSTALL:
	cp $(SRC)/unique   $(INSTDIR)/bin
	cp $(SRC)/breakout $(INSTDIR)/bin
	cp $(SRC)/pattern_detection $(INSTDIR)/bin
	cp $(SRC)/lda      $(INSTDIR)/bin
	cp $(SRC)/alda     $(INSTDIR)/bin
	cp $(SRC)/bseqr    $(INSTDIR)/bin
	cp $(SRC)/loess    $(INSTDIR)/bin
	cp $(SRC)/pmf      $(INSTDIR)/bin
	cp $(SRC)/bmf      $(INSTDIR)/bin
	cp $(SRC)/fits     $(INSTDIR)/bin
	cp $(SRC)/lr       $(INSTDIR)/bin
	cp $(SRC)/pr       $(INSTDIR)/bin
	cp $(SRC)/gr       $(INSTDIR)/bin
	cp $(SRC)/er       $(INSTDIR)/bin
	cp $(SRC)/tot      $(INSTDIR)/bin
	cp $(SRC)/totg     $(INSTDIR)/bin
	cp $(SRC)/blda     $(INSTDIR)/bin
	cp $(SRC)/jlda     $(INSTDIR)/bin

LIB_INSTALL:
	cp $(TS)/libedms.a $(INSTDIR)/lib
	cp $(TS)/libhpats.a         $(INSTDIR)/lib
	cp $(TS)/libloesss.a        $(INSTDIR)/lib
	cp $(TS)/libfitnts.a        $(INSTDIR)/lib
	cp $(UTILDIR)/libstrs.a     $(INSTDIR)/lib

INC_INSTALL:
	cp $(TS)/*.h     $(INSTDIR)/include
	cp $(DM)/*.h     $(INSTDIR)/include
	cp $(UTILDIR)/*.h   $(INSTDIR)/include

DOC_INSTALL:
	cp data/*          $(INSTDIR)/data
	cp doc/*           $(INSTDIR)/doc

install:
	-mkdir -p $(INSTDIR)/lib
	-mkdir -p $(INSTDIR)/include
	-mkdir -p $(INSTDIR)/bin
	-mkdir -p $(INSTDIR)/doc
	-mkdir -p $(INSTDIR)/data
	$(MAKE) APP_INSTALL
	$(MAKE) LIB_INSTALL
	$(MAKE) INC_INSTALL
	$(MAKE) DOC_INSTALL

#-----------------------------------------------------------------------
# Clean up
#-----------------------------------------------------------------------

.PHONY : localclean clean
localclean:
	rm -f *.o *~ *.flc core $(PRGS)

clean:
	$(MAKE) localclean
	cd $(UTILDIR);  $(MAKE) clean
	cd $(TS);       $(MAKE) clean
	cd $(DM);       $(MAKE) clean
	cd $(SRC);      $(MAKE) clean
