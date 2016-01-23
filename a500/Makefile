CPPFLAGS = -Isys

BINS	= stripes lines parseiff play8svx showilbm sprites4 sprites16 topaz8 \
          playahx playp61 wireframe kbtest shapes mouse transparency anim \
          uvmap metaballs blurred circles floor-old parselwo neons flatshade \
          plotter highway textscroll uvmap-rgb bumpmap-rgb glitch floor ball \
          tilezoomer twister-rgb showpchg blurred3d thunders credits parsepng \
          loadhunk showpng ExitUAE 

all:	startup.o libs $(addsuffix .exe, $(BINS))

ExitUAE: CC += -fbaserel -s
ExitUAE: ExitUAE.o

anim.exe:         anim.o startup.o libsys.a
ball.exe:         ball.o startup.o libsys.a
blurred.exe:      blurred.o startup.o libsys.a
blurred3d.exe:    blurred3d.o startup.o libsys.a
bumpmap-rgb.exe:  bumpmap-rgb.o startup.o libsys.a
circles.exe:      circles.o startup.o libsys.a
credits.exe:      credits.o startup.o libsys.a
neons.exe:        neons.o startup.o libsys.a
flatshade.exe:    flatshade.o startup.o libsys.a
floor.exe:        floor.o startup.o libsys.a
floor-old.exe:    floor-old.o startup.o libsys.a
highway.exe:      highway.o startup.o libsys.a
glitch.exe:       glitch.o startup.o libsys.a
lines.exe:        lines.o startup.o libsys.a
loadhunk.exe:     loadhunk.o libsys.a
metaballs.exe:    metaballs.o startup.o libsys.a
mouse.exe:        mouse.o startup.o libsys.a
kbtest.exe:       kbtest.o startup.o libsys.a
parseiff.exe:     parseiff.o libsys.a
parselwo.exe:     parselwo.o libsys.a
parsepng.exe:     parsepng.o libsys.a
play8svx.exe:     play8svx.o libsys.a
playahx.exe:      playahx.o startup.o libsys.a libahx.a
playp61.exe:      playp61.o startup.o libsys.a libp61.a
plotter.exe:      plotter.o startup.o libsys.a
shapes.exe:       shapes.o startup.o libsys.a
showilbm.exe:     showilbm.o startup.o libsys.a
showpchg.exe:     showpchg.o startup.o libsys.a
showpng.exe:      showpng.o startup.o libsys.a
sprites4.exe:     sprites4.o startup.o libsys.a
sprites16.exe:    sprites16.o startup.o libsys.a
stripes.exe:      stripes.o startup.o libsys.a
textscroll.exe:   textscroll.o startup.o libsys.a
tilezoomer.exe:   tilezoomer.o startup.o libsys.a
topaz8.exe:       topaz8.o startup.o libsys.a
transparency.exe: transparency.o startup.o libsys.a
thunders.exe:     thunders.o startup.o libsys.a
twister-rgb.exe:  twister-rgb.o startup.o libsys.a
uvmap.exe:        uvmap.o startup.o libsys.a
uvmap-rgb.exe:    uvmap-rgb.o startup.o libsys.a
wireframe.exe:    wireframe.o startup.o libsys.a

SUBDIRS = ahx p61 sys

libs:
	@for subdir in $(SUBDIRS); do $(MAKE) -C $$subdir || exit 1; done

clean-libs:
	@for subdir in $(SUBDIRS); do $(MAKE) -C $$subdir clean; done

clean::	clean-libs
	rm -f *.7z

archive:
	7z a "a500-$$(date +%F-%H%M).7z" $(BINS) data

include Makefile.common

.PHONY:	libs clean-libs

# vim: ts=8 sw=8 expandtab
