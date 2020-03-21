TOPDIR := $(realpath ../..)

SOURCES_GEN := data/ghostown-logo.c data/transparency-bg.c

PNG2C.ghostown-logo := --bitmap logo,160x128x2 --palette logo_pal,4
PNG2C.transparency-bg := --bitmap background,320x256x3 --palette background_pal,8

include $(TOPDIR)/build.effect.mk
