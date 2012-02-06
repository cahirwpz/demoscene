TOPDIR	= $(CURDIR)

include $(TOPDIR)/Makefile.common

export TOPDIR

BINS	= tunnel vector2d
SUBDIRS	= p61 system gfx std

all: $(SUBDIRS) $(BINS)

p61:
	$(MAKE) -C $@

system:
	$(MAKE) -C $@

gfx:
	$(MAKE) -C $@

std:
	$(MAKE) -C $@

tunnel: startup_effect.o tunnel.o tunnel_res.o distortion.o distortion_opt.o \
	frame_tools.o p61/p61.lib system/system.lib gfx/gfx.lib std/std.lib
	$(CC) $(CFLAGS) $(LIBS) -o $@ $^

vector2d: startup_effect.o vector2d.o vector2d_res.o frame_tools.o \
	system/system.lib gfx/gfx.lib std/std.lib
	$(CC) $(CFLAGS) $(LIBS) -o $@ $^

tunnel.o: tunnel.c
distortion.o: distortion.c distortion.h system/memory.h
distortion_opt.o: distortion_opt.s distortion.h

clean:
	$(MAKE) -C p61 clean
	$(MAKE) -C system clean
	$(MAKE) -C gfx clean
	$(MAKE) -C std clean
	$(RM) *~ *.o $(BINS)

.PHONY: all p61 system gfx std

# vim: sw=8 ts=8
