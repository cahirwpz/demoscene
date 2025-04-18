TOPDIR := $(realpath ../..)

RLE2PNG = rle2png.py
ELECTRONS2C = electrons2c.py

data/%.png: data/%.rle
	@echo "[RLE] $(DIR)$< -> $(DIR)$@"
	$(PYTHON3) $(RLE2PNG) $< $@ || (rm -f $@ && exit 1)

data/%-electrons.c: data/%-electrons.png
	@echo "[PNG] $(DIR)$< -> $(DIR)$@"
	$(PYTHON3) $(ELECTRONS2C) $< $@ || (rm -f $@ && exit 1)

CLEAN-FILES := data/p46basedprng.c data/cell-gradient.c \
	data/logo-electrons.c data/electric-lifeforms.c

PNG2C.p46basedprng := --bitmap p46basedprng,156x75x1
PNG2C.electric-lifeforms := --bitmap logo,160x128x1
PNG2C.cell-gradient := --pixmap gradient,4x64x12
ELECTRONS2C.logo-electrons := data/logo-electrons.c
SOURCES := game-of-life.c

include $(TOPDIR)/build/effect.mk
