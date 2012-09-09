TOPDIR	= $(CURDIR)

include $(TOPDIR)/Makefile.common

export TOPDIR

BINS	= flares particles raytunnel tunnel vector2d object3d playaudio colormap deformations
SUBDIRS	= p61 system gfx std engine txtgen distort tools audio

LIBS := $(foreach dir,$(SUBDIRS),-L$(TOPDIR)/$(dir)) -lgfx -ltools -lsystem -lstd $(LIBS)

all: libs $(BINS)

libs:
	for subdir in $(SUBDIRS); do $(MAKE) -C $$subdir; done

flares: startup_effect.o flares.o
	$(CC) $(CFLAGS) -o $@ $^ -ltxtgen $(LIBS)

particles: startup_effect.o particles.o
	$(CC) $(CFLAGS) -o $@ $^ -ltxtgen -lengine $(LIBS)

raytunnel: startup_effect.o raytunnel.o
	$(CC) $(CFLAGS) -o $@ $^ -ldistort -lengine $(LIBS)

tunnel: startup_effect.o tunnel.o
	$(CC) $(CFLAGS) -o $@ $^ -lp61 -ldistort -laudio $(LIBS)

vector2d: startup_effect.o vector2d.o
	$(CC) $(CFLAGS) -o $@ $^ $(LIBS)

object3d: startup_effect.o object3d.o
	$(CC) $(CFLAGS) -o $@ $^ -lengine $(LIBS)

playaudio: startup_effect.o playaudio.o
	$(CC) $(CFLAGS) -o $@ $^ -laudio $(LIBS)

colormap: startup_effect.o colormap.o
	$(CC) $(CFLAGS) -o $@ $^ -ldistort $(LIBS)

deformations: startup_effect.o deformations.o
	$(CC) $(CFLAGS) -o $@ $^ -ldistort $(LIBS)

deformations.o: deformations.c
	$(CC) $(CFLAGS) -Wno-unused $(OPTFLAGS) -c -o $@ $<

archive:
	7z a "bins-$$(date +%F).7z" $(BINS) data

clean:
	for subdir in $(SUBDIRS); do $(MAKE) -C $$subdir clean; done
	$(RM) *~ *.o $(BINS)

.PHONY: all archive libs $(BINS)

# vim: sw=8 ts=8
