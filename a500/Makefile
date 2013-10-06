BINS	= copper parseiff play8svx showilbm sprites4

all:	$(BINS)

copper:   startup.o copper.o coplist.o hardware.o vbr.o print.o
parseiff: parseiff.o iff.o print.o
play8svx: play8svx.o iff.o sound.o 8svx.o vbr.o hardware.o
showilbm: startup.o showilbm.o coplist.o iff.o ilbm.o gfx.o vbr.o print.o \
	  hardware.o
sprites4: startup.o sprites4.o coplist.o hardware.o iff.o ilbm.o gfx.o \
	  vbr.o print.o sprite.o

clean::
	rm -f $(BINS)

include Makefile.common

# vim: ts=8:sw=8
