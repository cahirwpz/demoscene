TOPDIR = $(realpath $(CURDIR)/..)

OBJS := colormap.o deformations.o flares.o object3d.o particles.o playaudio.o \
	raytunnel.o tunnel.o vector2d.o startup.o
BINS := colormap deformations flares object3d particles playaudio raytunnel \
	tunnel vector2d
LIBS = $(EXTRALIBS) -lgfx -ltools -lsystem -lstd

all:: $(BINS)

colormap: EXTRALIBS := -ldistort
colormap: startup.o colormap.o

deformations: EXTRALIBS := -ldistort
deformations: CFLAGS += -Wno-unused
deformations: startup.o deformations.o

flares: EXTRALIBS := -ltxtgen
flares: startup.o flares.o

object3d: EXTRALIBS := -lengine
object3d: startup.o object3d.o

playaudio: EXTRALIBS := -laudio
playaudio: startup.o playaudio.o

particles: EXTRALIBS := -ltxtgen -lengine
particles: startup.o particles.o

raytunnel: EXTRALIBS := -ldistort -lengine
raytunnel: startup.o raytunnel.o

tunnel: EXTRALIBS := -lp61 -ldistort -laudio
tunnel: startup.o tunnel.o

vector2d: startup.o vector2d.o

archive:
	7z a "bins-$$(date +%F).7z" $(BINS) data

clean::
	@$(RM) -v $(BINS)

include $(TOPDIR)/Makefile.common

.PHONY: all archive

# vim: sw=8 ts=8
