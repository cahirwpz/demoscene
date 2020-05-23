TOPDIR := $(realpath ../..)

CLEAN-FILES := data/blurred-pal-1.c data/blurred-pal-2.c data/blurred-clip.c

PNG2C.blurred-clip := --bitmap clip,160x128x4
PNG2C.blurred-pal-1 := --palette blurred_1_pal,32
PNG2C.blurred-pal-2 := --palette blurred_2_pal,32

include $(TOPDIR)/build/effect.mk
