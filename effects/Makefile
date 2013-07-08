TOPDIR = $(realpath $(CURDIR)/..)

OBJS := blur.o colormap.o deformations.o flares.o layers.o object3d.o \
	particles.o playaudio.o raytunnel.o tunnel.o vector2d.o startup.o
BINS := blur colormap deformations flares layers object3d particles playaudio \
	raytunnel tunnel vector2d
LIBS := libgfx.a libtools.a libsystem.a libstd.a

all:: $(BINS)

colormap: startup.o colormap.o libdistort.a $(LIBS)

deformations: CFLAGS += -Wno-unused
deformations: startup.o deformations.o libdistort.a $(LIBS)

blur: startup.o blur.o $(LIBS)
flares: startup.o flares.o libtxtgen.a $(LIBS)
layers: startup.o layers.o $(LIBS)
object3d: startup.o object3d.o libengine.a $(LIBS)
playaudio: startup.o playaudio.o libaudio.a $(LIBS)
particles: startup.o particles.o libtxtgen.a libengine.a $(LIBS)
raytunnel: startup.o raytunnel.o libdistort.a libengine.a $(LIBS)
tunnel: startup.o tunnel.o libp61.a libdistort.a libaudio.a $(LIBS)
vector2d: startup.o vector2d.o $(LIBS)

archive:
	7z a "bins-$$(date +%F).7z" $(BINS) data

clean::
	@$(RM) -v $(BINS)

include $(TOPDIR)/Makefile.common

.PHONY: all archive

# vim: sw=8 ts=8
