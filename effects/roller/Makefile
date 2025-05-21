TOPDIR := $(realpath ../..)

CLEAN-FILES := data/roller-bg.c data/magland16.c

PNG2C.roller-bg := --palette roller_pal,32  --bitmap roller_bp,320x256x4,+displayable
PNG2C.magland16 := --palette texture_pal,16 --pixmap texture_bp,16x16x8

include $(TOPDIR)/build/effect.mk

