DATA := MagicLand.h MagicLand-tiles.ilbm

all: $(DATA)

MagicLand-tiles.png: MagicLand.h
MagicLand-tiles.ilbm: MagicLand-tiles.png

include ../../build.mk

clean::
	$(RM) $(DATA) MagicLand-tiles.png MagicLand-map.bin
