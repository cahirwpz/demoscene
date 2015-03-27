CPPFLAGS = -Isys

BINS	= stripes lines parseiff play8svx showilbm sprites4 sprites16 topaz8 \
          playahx playp61 wireframe kbtest shapes mouse transparency anim \
          uvmap metaballs blurred circles floor-old parselwo neons flatshade \
          plotter highway textscroll uvmap-rgb bumpmap-rgb glitch floor ball \
          tilezoomer twister-rgb showpchg blurred3d thunders

all:	startup.o libs $(BINS)

anim:           anim.o startup.o libsys.a
ball:           ball.o startup.o libsys.a
blurred:        blurred.o startup.o libsys.a
blurred3d:      blurred3d.o startup.o libsys.a
bumpmap-rgb:    bumpmap-rgb.o startup.o libsys.a
circles:        circles.o startup.o libsys.a
neons:          neons.o startup.o libsys.a
flatshade:      flatshade.o startup.o libsys.a
floor:          floor.o startup.o libsys.a
floor-old:      floor-old.o startup.o libsys.a
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
showpchg:       showpchg.o startup.o libsys.a
sprites4:       sprites4.o startup.o libsys.a
sprites16:      sprites16.o startup.o libsys.a
stripes:        stripes.o startup.o libsys.a
textscroll:     textscroll.o startup.o libsys.a
tilezoomer:     tilezoomer.o startup.o libsys.a
topaz8:         topaz8.o startup.o libsys.a
transparency:   transparency.o startup.o libsys.a
thunders:       thunders.o startup.o libsys.a
twister-rgb:    twister-rgb.o startup.o libsys.a
uvmap:          uvmap.o startup.o libsys.a
uvmap-rgb:      uvmap-rgb.o startup.o libsys.a
wireframe:      wireframe.o startup.o libsys.a

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
