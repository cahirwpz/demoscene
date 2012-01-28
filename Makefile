TOPDIR	= $(CURDIR)

include $(TOPDIR)/Makefile.common

export TOPDIR

BINS	= main
SUBDIRS	= p61
OBJS	= $(patsubst %.c, %.o, $(wildcard *.c))

all: $(SUBDIRS) $(BINS)

p61:
	$(MAKE) -C p61

main: $(OBJS) p61/p61.o c2p1x1_8_c5_bm.o distortion_opt.o
	$(CC) $(CFLAGS) -ldebug -lamigas -o $@ $^

main.o:	main.c
input.o: input.c input.h common.h
vblank.o: vblank.c vblank.h
display.o: display.c display.h common.h
fileio.o: fileio.c fileio.h
c2p1x1_8_c5_bm.o: c2p1x1_8_c5_bm.s c2p.h
distortion.o: distortion.c distortion.h common.h
distortion_opt.o: distortion_opt.s distortion.h
resource.o: resource.c resource.h resource_internal.h

clean:
	$(MAKE) -C p61 clean
	$(RM) *~ *.o $(BINS)

.PHONY: all p61

# vim: sw=8 ts=8
