TOPDIR = $(CURDIR)/..

include $(TOPDIR)/Makefile.common

OBJS = c2p1x1_8_c5_bm.o check.o display.o fileio.o input.o vblank.o

libsystem.a: $(OBJS)

c2p1x1_8_c5_bm.o: c2p1x1_8_c5_bm.s c2p.h
check.o: check.c check.h
display.o: display.c display.h memory.h
fileio.o: fileio.c fileio.h
input.o: input.c input.h debug.h memory.h
vblank.o: vblank.c vblank.h

clean:
	$(RM) *.a *.o *~

.PHONY: clean
