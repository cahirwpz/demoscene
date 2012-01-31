TOPDIR = ..

include $(TOPDIR)/Makefile.common

OBJS = c2p1x1_8_c5_bm.o display.o fileio.o input.o resource.o vblank.o

all: system.lib

c2p1x1_8_c5_bm.o: c2p1x1_8_c5_bm.s c2p.h
display.o: display.c display.h memory.h
fileio.o: fileio.c fileio.h
input.o: input.c input.h debug.h memory.h
resource.o: resource.c resource.h resource_internal.h debug.h memory.h
vblank.o: vblank.c vblank.h

system.lib: $(OBJS)
	cat $^ > $@

clean:
	$(RM) *.lib *.o *~

.PHONY: clean
