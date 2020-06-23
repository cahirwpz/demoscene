TOPDIR := $(realpath ../..)

SOURCES := butterfly-gears.c copper-routines.s

CLEAN-FILES := \
	data/ball_small.c \
	data/ball_large.c \
	data/book_bottom.c \
	data/texture_butterfly.c \
	data/textureloop-generated.s

PNG2C.ball_small := --bitmap ball_small,320x64x5
PNG2C.ball_large := --bitmap ball_large,320x112x5
PNG2C.book_bottom := --bitmap book_bottom,320x51x4 --palette book_pal,16
PNG2C.texture_butterfly := --pixmap texture_butterfly,128x256x12

include $(TOPDIR)/build.effect.mk

copper-routines.s: data/textureloop-generated.s

%.s: %.txt
	@echo "[GEN] $(DIR)$< -> $(DIR)$@"
	$(PYTHON3) data/gen-unrolled-loop.py $< > $@
