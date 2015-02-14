TOPDIR = $(realpath $(CURDIR)/..)

OBJS := blur.o colormap.o flares.o layers.o object3d.o particles.o \
	playaudio.o raytunnel.o tunnel.o uvmap.o uvmap-blit.o uvmap-compose.o \
	uvmap-light.o vector2d.o startup.o fire.o bumpmap.o polar.o flow.o
BINS := blur colormap flares flat-shading layers object3d particles playaudio \
	raytunnel tunnel uvmap uvmap-blit uvmap-compose uvmap-light vector2d \
	fire bumpmap polar flow
LIBS := libgfx.a libtools.a libsystem.a libstd.a libtinf.a

all:: $(BINS)

colormap: startup.o colormap.o $(LIBS)

blur: startup.o blur.o $(LIBS)
bumpmap: startup.o bumpmap.o bumpmap-opt.o libuvmap.a $(LIBS)
fire: startup.o fire-opt.o fire.o $(LIBS)
flares: startup.o flares.o libtxtgen.a $(LIBS)
flat-shading: startup.o flat-shading.o libuvmap.a libengine.a $(LIBS)
flow: startup.o flow.o $(LIBS)
layers: startup.o layers.o $(LIBS)
object3d: startup.o object3d.o libengine.a $(LIBS)
particles: startup.o particles.o libtxtgen.a libengine.a $(LIBS)
playaudio: startup.o playaudio.o libaudio.a libuvmap.a $(LIBS)
polar: startup.o polar.o libuvmap.a $(LIBS)
raytunnel: startup.o raytunnel.o libuvmap.a libengine.a $(LIBS)
tunnel: startup.o tunnel.o libuvmap.a libaudio.a $(LIBS)
uvmap-blit: startup.o uvmap-blit.o libuvmap.a libtxtgen.a $(LIBS)
uvmap-compose: startup.o uvmap-compose.o libuvmap.a $(LIBS)
uvmap-light: startup.o uvmap-light.o libuvmap.a $(LIBS)
uvmap: startup.o uvmap.o libtxtgen.a libuvmap.a $(LIBS)
vector2d: startup.o vector2d.o $(LIBS)

archive:
	7z a "all-$$(date +%F-%H%M).7z" $(BINS) data

binaries:
	7z a "bin-$$(date +%F-%H%M).7z" $(BINS)

clean::
	@$(RM) -v $(BINS) *.7z
	@$(RM) -rvf binaries

include $(TOPDIR)/Makefile.common

.PHONY: all archive

# vim: sw=8 ts=8
