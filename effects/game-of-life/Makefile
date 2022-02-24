TOPDIR := $(realpath ../..)

RLE2PNG = rle2png.py
data/%.png: data/%.rle
	$(PYTHON3) $(RLE2PNG) data/p46basedprng.rle data/p46basedprng.png
	$(PYTHON3) $(RLE2PNG) --rotate 270 data/weekenders.rle data/weekenders.png

CLEAN-FILES := data/current-board.c data/p46basedprng.c data/weekenders.c data/beam.c
PNG2C.current-board := --bitmap current_board,192x130x1
PNG2C.p46basedprng := --bitmap p46basedprng,156x75x1
PNG2C.weekenders := --bitmap weekenders,24x37x1
PNG2C.beam := --sprite laser_beam,256,1 --palette laser_beam_pal,4
SOURCES := game-of-life.c

include $(TOPDIR)/build/effect.mk
