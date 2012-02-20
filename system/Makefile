TOPDIR = $(CURDIR)/..

include $(TOPDIR)/Makefile.common

OBJS = c2p1x1_8_c5_bm.o check.o display.o fileio.o input.o lock.o vblank.o

libsystem.a: $(OBJS)

c2p1x1_8_c5_bm.o: c2p1x1_8_c5_bm.s c2p.h
check.o: check.c check.h
display.o: display.c display.h
fileio.o: fileio.c fileio.h
input.o: input.c input.h
lock.o: lock.c lock.h
vblank.o: vblank.c vblank.h

clean:
	$(RM) *.a *.o *~

.PHONY: clean
