BINS	= copper lines parseiff play8svx showilbm sprites4 sprites16 topaz8

all:	$(BINS)

copper:    startup.o copper.o coplist.o hardware.o vbr.o print.o
lines:	   startup.o lines.o coplist.o hardware.o gfx.o print.o vbr.o blitter.o
parseiff:  parseiff.o iff.o print.o
play8svx:  play8svx.o iff.o sound.o 8svx.o vbr.o hardware.o
showilbm:  startup.o showilbm.o coplist.o iff.o ilbm.o gfx.o vbr.o print.o \
	   hardware.o
sprites4:  startup.o sprites4.o coplist.o hardware.o iff.o ilbm.o gfx.o \
	   vbr.o print.o sprite.o
sprites16: startup.o sprites16.o coplist.o hardware.o iff.o ilbm.o gfx.o \
	   vbr.o print.o sprite.o
topaz8:	   startup.o topaz8.o coplist.o hardware.o gfx.o vbr.o print.o

clean::
	rm -f $(BINS) *.7z

archive:
	7z a "a500-$$(date +%F-%H%M).7z" $(BINS) data

include Makefile.common

# vim: ts=8:sw=8
