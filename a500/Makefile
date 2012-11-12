CC	= m68k-amigaos-gcc -noixemul
AS	= vasm -quiet
CFLAGS	= $(LDFLAGS) $(OFLAGS) $(WFLAGS)

ASFLAGS	= -Faout -phxass -m68010
LDFLAGS	= -m68000 -msmall-code 
OFLAGS	= -O2 -fomit-frame-pointer
WFLAGS	= -Wall

all:	copper iff

copper:	startup.o copper.o coplist.o hardware.o vbr.o print.o
iff:	iff.o print.o

%.s: %.c
	$(CC) $(CFLAGS) -fverbose-asm -S -o $@ $<

clean:
	rm -f *~ *.o
	rm -f copper iff

# vim: ts=8:sw=8
