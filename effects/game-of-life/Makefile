TOPDIR := $(realpath ../..)

RLE2PNG = rle2png.py
data/%.png: data/%.rle
	$(PYTHON3) $(RLE2PNG) data/p46basedprng.rle data/p46basedprng.png

CLEAN-FILES := data/current-board.c data/p46basedprng.c
PNG2C.p46basedprng := --bitmap p46basedprng,156x75x1
SOURCES := game-of-life.c

include $(TOPDIR)/build/effect.mk
