TOPDIR := $(realpath ../..)

RLE2PNG = rle2png.py
data/%.png: data/%.rle
	$(PYTHON3) $(RLE2PNG) data/p46basedprng.rle data/p46basedprng.png

CLEAN-FILES := data/initial-board.c data/p46basedprng.c  data/run.c data/electrons.c data/tails.c data/cell-gradient.c
PNG2C.p46basedprng := --bitmap p46basedprng,156x75x1
PNG2C.electrons := --bitmap electrons,160x128x1
PNG2C.tails := --bitmap tails,160x128x1
PNG2C.run := --bitmap run,160x128x1
PNG2C.cell-gradient := --pixmap gradient,4x64x12
SOURCES := game-of-life.c

include $(TOPDIR)/build/effect.mk
