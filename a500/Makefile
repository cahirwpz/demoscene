CC	= m68k-amigaos-gcc -noixemul
AS	= vasm -quiet
CFLAGS	= $(LDFLAGS) $(OFLAGS) $(WFLAGS) $(DFLAGS)

ASFLAGS	= -Faout -phxass -m68010
LDFLAGS	= -m68000 -msmall-code 
OFLAGS	= -O2 -fomit-frame-pointer
WFLAGS	= -Wall

# Don't reload library base for each call.
DFLAGS	= -D__CONSTLIBBASEDECL__=const

# libnix13.a contains a few functions that don't depend on utility.library
# which is not present in Kickstart 1.3
LDLIBS	= -lnix13

all:	copper parseiff play8svx showilbm

copper:	startup.o copper.o coplist.o hardware.o vbr.o print.o
parseiff: parseiff.o iff.o print.o
play8svx: play8svx.o iff.o sound.o 8svx.o vbr.o hardware.o
showilbm: startup.o showilbm.o coplist.o iff.o ilbm.o gfx.o vbr.o print.o \
	  hardware.o

%.s: %.c
	$(CC) $(CFLAGS) -fverbose-asm -S -o $@ $<

clean:
	rm -f *~ *.o
	rm -f copper parseiff play8svx showilbm

# vim: ts=8:sw=8
