TOPDIR = $(realpath $(CURDIR)/..)

OBJS := blur.o colormap.o flares.o layers.o object3d.o particles.o \
	playaudio.o raytunnel.o tunnel.o uvmap.o uvmap-blit.o uvmap-compose.o \
	uvmap-light.o vector2d.o startup.o
BINS := blur colormap flares flat-shading layers object3d particles playaudio \
	raytunnel tunnel uvmap uvmap-blit uvmap-compose uvmap-light vector2d 
LIBS := libgfx.a libtools.a libsystem.a libstd.a

all:: $(BINS)

colormap: startup.o colormap.o $(LIBS)

uvmap: CFLAGS += -Wno-unused
uvmap: startup.o uvmap.o libtxtgen.a libuvmap.a $(LIBS)
uvmap-blit: CFLAGS += -Wno-unused
uvmap-blit: startup.o uvmap-blit.o libuvmap.a libtxtgen.a $(LIBS)
uvmap-light: CFLAGS += -Wno-unused
uvmap-light: startup.o uvmap-light.o libuvmap.a $(LIBS)
uvmap-compose: CFLAGS += -Wno-unused
uvmap-compose: startup.o uvmap-compose.o libuvmap.a $(LIBS)
flat-shading: CFLAGS += -Wno-unused
flat-shading: startup.o flat-shading.o libuvmap.a libengine.a $(LIBS)

blur: startup.o blur.o $(LIBS)
flares: startup.o flares.o libtxtgen.a $(LIBS)
layers: startup.o layers.o $(LIBS)
object3d: startup.o object3d.o libengine.a $(LIBS)
playaudio: startup.o playaudio.o libaudio.a $(LIBS)
particles: startup.o particles.o libtxtgen.a libengine.a $(LIBS)
raytunnel: startup.o raytunnel.o libuvmap.a libengine.a $(LIBS)
tunnel: startup.o tunnel.o libp61.a libuvmap.a libaudio.a $(LIBS)
vector2d: startup.o vector2d.o $(LIBS)

archive:
	7z a "all-$$(date +%F).7z" $(BINS) data

binaries:
	7z a "bin-$$(date +%F).7z" $(BINS)

clean::
	@$(RM) -v $(BINS) *.7z
	@$(RM) -rvf binaries

include $(TOPDIR)/Makefile.common

.PHONY: all archive

# vim: sw=8 ts=8
