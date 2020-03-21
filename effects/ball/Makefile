TOPDIR := $(realpath ../..)

SOURCES_GEN := data/dragon-bg.c
DATA := data/texture-15.png data/ball.bin

include $(TOPDIR)/build.effect.mk

data/dragon-bg.c: PNG2C += --bitmap background,320x256x4 \
	                   --palette background_pal,16
