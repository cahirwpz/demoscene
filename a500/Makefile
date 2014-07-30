CPPFLAGS = -Isys

BINS	= copper lines parseiff play8svx showilbm sprites4 sprites16 topaz8 \
	  playahx playp61 cube kbtest shapes chunky-16c chunky-rgb

all:	startup.o libs $(BINS)

chunky-16c:     chunky-16c.o startup.o libsys.a
chunky-rgb:     chunky-rgb.o startup.o libsys.a
copper:         copper.o cophack.o startup.o libsys.a
cube:           cube.o 3d.o sincos.o startup.o libsys.a
lines:          lines.o startup.o libsys.a
kbtest:         kbtest.o startup.o libsys.a
parseiff:       parseiff.o libsys.a
play8svx:       play8svx.o libsys.a
playahx:        playahx.o startup.o libsys.a libahx.a
playp61:        playp61.o startup.o libsys.a libp61.a
shapes:         shapes.o 2d.o sincos.o startup.o libsys.a
showilbm:       showilbm.o startup.o libsys.a
sprites4:       sprites4.o startup.o libsys.a
sprites16:      sprites16.o startup.o libsys.a
topaz8:         topaz8.o startup.o libsys.a

sincos.s: sincos.py
	python sincos.py

SUBDIRS = ahx p61 sys

libs:
	@for subdir in $(SUBDIRS); do $(MAKE) -C $$subdir || exit 1; done

clean-libs:
	@for subdir in $(SUBDIRS); do $(MAKE) -C $$subdir clean; done

clean::	clean-libs
	rm -vf sincos.s
	rm -f $(BINS) *.7z

archive:
	7z a "a500-$$(date +%F-%H%M).7z" $(BINS) data

include Makefile.common

.PHONY:	libs clean-libs

# vim: ts=8 sw=8 expandtab
