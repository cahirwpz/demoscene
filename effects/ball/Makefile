TOPDIR := $(realpath ../..)

CLEAN-FILES := data/dragon-bg.c data/texture-15.c
DATA := data/ball.bin

PNG2C.dragon-bg := --bitmap background,320x256x4 --palette background_pal,16
PNG2C.texture-15 := --pixmap texture,128x128x4 --palette texture_pal,16

include $(TOPDIR)/build.effect.mk
