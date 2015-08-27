UTIL     = ../util
DM       = ../dm
TS       = ../ts

LIBS     = -L../util -L../ts -L../dm
INC      = -I../util -I../ts -I../dm

CC       = gcc -std=c99
CFBASE   = -Wall -pedantic -pipe
CFLAGS   = $(CFBASE) -DNDEBUG -DNPRINT -O3

PRGS     = breakout          \
		   pattern_detection \
		   unique            \
		   bseqr             \
		   viterbi           \
		   pmf               \
		   bmf               \
		   loess             \
		   fits              \
           lr                \
           pr                \
           gr                \
           er                \
		   lda               \
		   alda              \
		   tot               \
		   totg              \
		   jlda              \
		   blda

all  : $(PRGS)


# -----------------------------------------
# objs
# -----------------------------------------

lda.o        : lda.c 
	$(CC) $(CFLAGS) $(INC) -c $^ -o $@

author_lda.o : author_lda.c
	$(CC) $(CFLAGS) $(INC) -c $^ -o $@

tot.o        : tot.c
	$(CC) $(CFLAGS) $(INC) -c $^ -o $@

bri_lda.o    : bri_lda.c
	$(CC) $(CFLAGS) $(INC) -c $^ -o $@

joint_lda.o  : joint_lda.c
	$(CC) $(CFLAGS) $(INC) -c $^ -o $@

breakout.o   : breakout.c
	$(CC) $(CFLAGS) $(INC) -c $^ -o $@

unique.o     : unique.c
	$(CC) $(CFLAGS) $(INC) -c $^ -o $@

pattern_detection.o    : pattern_detection.c
	$(CC) $(CFLAGS) $(INC) -c $^ -o $@

bseqr.o      : bseqr.c
	$(CC) $(CFLAGS) $(INC) -c $^ -o $@

viterbi.o    : viterbi.c
	$(CC) $(CFLAGS) $(INC) -c $^ -o $@

loess.o      : loess.c
	$(CC) $(CFLAGS) $(INC) -c $^ -o $@

ts_fit.o      : fit_newton.c
	$(CC) $(CFLAGS) $(INC) -c $^ -o $@

lr.o          : lr.c
	$(CC) $(CFLAGS) $(INC) -c $^ -o $@

pr.o          : pr.c
	$(CC) $(CFLAGS) $(INC) -c $^ -o $@

gr.o          : gr.c
	$(CC) $(CFLAGS) $(INC) -c $^ -o $@

er.o          : er.c
	$(CC) $(CFLAGS) $(INC) -c $^ -o $@

pmf.o        : mf.c
	$(CC) $(CFLAGS) $(INC) -DPMF -c $^ -o $@

bmf.o        : mf.c
	$(CC) $(CFLAGS) $(INC) -DBMF -c $^ -o $@

# ----------------------------------------
#  bins
# ----------------------------------------

lda : lda.o 
	$(CC) $(LIBS)  $^ -lldas -o $@

alda : author_lda.o 
	$(CC) $(LIBS)  $^ -laldas -o $@

tot  : tot.o
	$(CC) $(LIBS)  $^ -ltots -lm -o $@

totg : tot.o
	$(CC) $(LIBS)  $^ -ltotgs -lm -o $@

jlda : joint_lda.o
	$(CC) $(LIBS)  $^ -ljldas -lm -o $@

blda: bri_lda.o
	$(CC) $(LIBS)  $^ -lbldas -o $@

breakout: breakout.o
	$(CC) $(LIBS)  $^ -ledms -o $@

pattern_detection : pattern_detection.o
	$(CC) $(LIBS)  $^ -lhpats -lm -o $@

unique : unique.o
	$(CC) $(LIBS)  $^ -lsets -o $@

bseqr  : bseqr.o
	$(CC) $(LIBS)  $^ -lmodls -lm -o $@

viterbi : viterbi.o
	$(CC) $(LIBS)  $^ -lvtbs -lm -o $@

loess   : loess.o
	$(CC) $(LIBS)  $^ -lloesss -lm -o $@

fits: ts_fit.o
	$(CC) $(LIBS)  $^ -lfitnts -lm -o $@

lr  :   lr.o
	$(CC) $(LIBS)  $^ -llrs -lm -o $@

pr  :   pr.o
	$(CC) $(LIBS)  $^ -lprs -lm -o $@

gr  :   gr.o
	$(CC) $(LIBS)  $^ -lgrs -lm -o $@

er  :   er.o
	$(CC) $(LIBS)  $^ -lers -lm -o $@

pmf     : pmf.o
	$(CC) $(LIBS)  $^ -lpmfs -lm -o $@

bmf     : bmf.o
	$(CC) $(LIBS)  $^ -lbmfs -lm -o $@

# ----------------------------------------
#  clean
# ----------------------------------------

.PHONY : clean

clean:
	rm -f  *.o *.so *.a $(PRGS)
