TOPDIR := $(realpath ../..)

SOURCES := butterfly-gears.c copper-routines.s

CLEAN-FILES := \
	data/ball_small.c \
	data/ball_large.c \
	data/book_bottom.c \
	data/texture_butterfly.c \
	data/textureloop-generated.s \
	data/spr0.c \
	data/spr1.c \
	data/spr2.c \
	data/spr3.c \
	data/spr4.c \
	data/spr5.c \
	data/spr6.c \
	data/spr7.c

PNG2C.ball_small := --bitmap ball_small,320x64x5
PNG2C.ball_large := --bitmap ball_large,320x112x5
PNG2C.book_bottom := --bitmap book_bottom,320x51x4 --palette book_pal,16
PNG2C.texture_butterfly := --pixmap texture_butterfly,128x256x12
PNG2C.spr0 := --bitmap spr0,16x144x2,+interleaved
PNG2C.spr1 := --bitmap spr1,16x144x2,+interleaved
PNG2C.spr2 := --bitmap spr2,16x144x2,+interleaved
PNG2C.spr3 := --bitmap spr3,16x144x2,+interleaved
PNG2C.spr4 := --bitmap spr4,16x8x2,+interleaved
PNG2C.spr5 := --bitmap spr5,16x8x2,+interleaved
PNG2C.spr6 := --bitmap spr6,16x8x2,+interleaved
PNG2C.spr7 := --bitmap spr7,16x8x2,+interleaved

include $(TOPDIR)/build.effect.mk

copper-routines.s: data/textureloop-generated.s

%.s: %.txt
	@echo "[GEN] $(DIR)$< -> $(DIR)$@"
	$(PYTHON3) data/gen-unrolled-loop.py $< > $@
