TOPDIR	= $(CURDIR)

include $(TOPDIR)/Makefile.common

export TOPDIR

BINS	= tunnel vector2d object3d
SUBDIRS	= p61 system gfx std engine txtgen distort

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
txtgen:
	$(MAKE) -C $@
distort:
	$(MAKE) -C $@

tunnel: startup_effect.o tunnel.o frame_tools.o
	$(CC) $(CFLAGS) -o $@ $^ -lp61 -ldistort $(LIBS)

vector2d: startup_effect.o vector2d.o frame_tools.o
	$(CC) $(CFLAGS) -o $@ $^ $(LIBS)

object3d: startup_effect.o object3d.o frame_tools.o
	$(CC) $(CFLAGS) -o $@ $^ -lengine $(LIBS)

archive:
	7z a "bins-$$(date +%F).7z" $(BINS) data

clean:
	for subdir in $(SUBDIRS); do $(MAKE) -C $$subdir clean; done
	$(RM) *~ *.o $(BINS)

.PHONY: all 7z $(SUBDIRS) $(BINS)

# vim: sw=8 ts=8
