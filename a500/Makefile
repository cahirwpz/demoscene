CC      = m68k-amigaos-gcc -noixemul
CFLAGS   = $(LDFLAGS) $(OFLAGS) $(WFLAGS)

LDFLAGS = -m68000 -msmall-code 
OFLAGS  = -O2 -fomit-frame-pointer
WFLAGS  = -Wall

copper: copper.o
copper.o: copper.c

clean:
	rm -f *~ *.o
	rm -f copper
