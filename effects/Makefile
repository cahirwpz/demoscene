TOPDIR = $(CURDIR)/..

include $(TOPDIR)/Makefile.common

BINS = colormap deformations flares object3d particles playaudio raytunnel tunnel vector2d

LIBS := $(LIBDIR) -lgfx -ltools -lsystem -lstd $(LIBS)

all: $(BINS)

colormap: startup.o colormap.o
	$(CC) $(CFLAGS) -o $@ $^ -ldistort $(LIBS)

deformations: startup.o deformations.o
	$(CC) $(CFLAGS) -o $@ $^ -ldistort $(LIBS)

deformations.o: deformations.c
	$(CC) $(CFLAGS) -Wno-unused $(OPTFLAGS) -c -o $@ $<

flares: startup.o flares.o
	$(CC) $(CFLAGS) -o $@ $^ -ltxtgen $(LIBS)

object3d: startup.o object3d.o
	$(CC) $(CFLAGS) -o $@ $^ -lengine $(LIBS)

playaudio: startup.o playaudio.o
	$(CC) $(CFLAGS) -o $@ $^ -laudio $(LIBS)

particles: startup.o particles.o
	$(CC) $(CFLAGS) -o $@ $^ -ltxtgen -lengine $(LIBS)

raytunnel: startup.o raytunnel.o
	$(CC) $(CFLAGS) -o $@ $^ -ldistort -lengine $(LIBS)

tunnel: startup.o tunnel.o
	$(CC) $(CFLAGS) -o $@ $^ -lp61 -ldistort -laudio $(LIBS)

vector2d: startup.o vector2d.o
	$(CC) $(CFLAGS) -o $@ $^ $(LIBS)

archive:
	7z a "bins-$$(date +%F).7z" $(BINS) data

clean:
	$(RM) *~ *.o $(BINS)

.PHONY: all archive $(BINS)

# vim: sw=8 ts=8
