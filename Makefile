TOPDIR	= $(CURDIR)

include $(TOPDIR)/Makefile.common

export TOPDIR

BINS	= tunnel vector2d object3d
SUBDIRS	= p61 system gfx std engine

LIBS := $(foreach dir,$(SUBDIRS),-L$(TOPDIR)/$(dir)) -lgfx -lsystem -lstd $(LIBS)

all: $(SUBDIRS) $(BINS)

p61:
	$(MAKE) -C $@
system:
	$(MAKE) -C $@
gfx:
	$(MAKE) -C $@
std:
	$(MAKE) -C $@
engine:
	$(MAKE) -C $@

tunnel: startup_effect.o tunnel.o res_tunnel.o distortion.o distortion_opt.o \
	frame_tools.o
	$(CC) $(CFLAGS) -o $@ $^ -lp61 $(LIBS)

vector2d: startup_effect.o vector2d.o res_vector2d.o frame_tools.o
	$(CC) $(CFLAGS) -o $@ $^ $(LIBS)

object3d: startup_effect.o object3d.o res_object3d.o frame_tools.o
	$(CC) $(CFLAGS) -o $@ $^ -lengine $(LIBS)

tunnel.o: tunnel.c
distortion.o: distortion.c distortion.h
distortion_opt.o: distortion_opt.s distortion.h

archive:
	7z a "bins-$$(date +%F).7z" $(BINS) data

clean:
	for subdir in $(SUBDIRS); do $(MAKE) -C $$subdir clean; done
	$(RM) *~ *.o $(BINS)

.PHONY: all 7z $(SUBDIRS) $(BINS)

# vim: sw=8 ts=8
