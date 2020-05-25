TOPDIR := $(realpath ../..)

SOURCES := butterfly-gears.c textureloop.s

CLEAN-FILES := \
	data/gears_testscreen.c \
	data/gears_testscreen_debug.c \
	data/texture_butterfly.c \
	data/texture_butterfly2.c \
	data/textureloop-generated.s

PNG2C.ball_small := --bitmap ball_small,320x60x5
PNG2C.ball_large := --bitmap ball_large,320x112x5
PNG2C.book_bottom := --bitmap book_bottom,320x51x4 --palette book_pal,16
PNG2C.texture_butterfly := --pixmap texture_butterfly,128x256x12

include $(TOPDIR)/build.effect.mk

textureloop.s: data/textureloop-generated.s

%.s: %.txt
	@echo "[GEN] $(DIR)$< -> $(DIR)$@"
	$(PYTHON3) data/gen-unrolled-loop.py $< > $@
