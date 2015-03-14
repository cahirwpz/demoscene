CPPFLAGS = -Isys

BINS	= copper lines parseiff play8svx showilbm sprites4 sprites16 topaz8 \
          playahx playp61 cube kbtest shapes chunky-16c chunky-rgb mouse \
          uvmap metaballs blurred circles floor parselwo showlwo neons \
          plotter highway textscroll uvmap-rgb bumpmap-rgb glitch \
          transparency tilezoomer twister-rgb anim showpchg

all:	startup.o libs $(BINS)

anim:           anim.o startup.o libsys.a
blurred:        blurred.o startup.o libsys.a
blurred3d:      blurred3d.o startup.o libsys.a
bumpmap-rgb:    bumpmap-rgb.o startup.o libsys.a
chunky-16c:     chunky-16c.o startup.o libsys.a
chunky-rgb:     chunky-rgb.o startup.o libsys.a
circles:        circles.o startup.o libsys.a
neons:          neons.o startup.o libsys.a
copper:         copper.o startup.o libsys.a
cube:           cube.o startup.o libsys.a
floor:          floor.o startup.o libsys.a
highway:        highway.o startup.o libsys.a
glitch:         glitch.o startup.o libsys.a
lines:          lines.o startup.o libsys.a
metaballs:      metaballs.o startup.o libsys.a
mouse:          mouse.o startup.o libsys.a
kbtest:         kbtest.o startup.o libsys.a
parseiff:       parseiff.o libsys.a
parselwo:       parselwo.o libsys.a
play8svx:       play8svx.o libsys.a
playahx:        playahx.o startup.o libsys.a libahx.a
playp61:        playp61.o startup.o libsys.a libp61.a
plotter:        plotter.o startup.o libsys.a
shapes:         shapes.o startup.o libsys.a
showilbm:       showilbm.o startup.o libsys.a
showlwo:        showlwo.o startup.o libsys.a
showpchg:       showpchg.o startup.o libsys.a
sprites4:       sprites4.o startup.o libsys.a
sprites16:      sprites16.o startup.o libsys.a
textscroll:     textscroll.o startup.o libsys.a
topaz8:         topaz8.o startup.o libsys.a
uvmap:          uvmap.o startup.o libsys.a
uvmap-rgb:      uvmap-rgb.o startup.o libsys.a
tilezoomer:     tilezoomer.o startup.o libsys.a
transparency:   transparency.o startup.o libsys.a
twister-rgb:    twister-rgb.o startup.o libsys.a

SUBDIRS = ahx p61 sys

libs:
	@for subdir in $(SUBDIRS); do $(MAKE) -C $$subdir || exit 1; done

clean-libs:
	@for subdir in $(SUBDIRS); do $(MAKE) -C $$subdir clean; done

clean::	clean-libs
	rm -vf sintab.s
	rm -f $(BINS) *.7z

archive:
	7z a "a500-$$(date +%F-%H%M).7z" $(BINS) data

include Makefile.common

.PHONY:	libs clean-libs

# vim: ts=8 sw=8 expandtab
