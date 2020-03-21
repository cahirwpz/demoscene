TOPDIR := $(realpath ../..)

SOURCES_GEN := data/ghostown-logo.c data/transparency-bg.c

include $(TOPDIR)/build.effect.mk

data/ghostown-logo.c: PNG2C += --bitmap logo,160x128x2 --palette logo_pal,4
data/transparency-bg.c: PNG2C += --bitmap background,320x256x3 --palette background_pal,8
