TOPDIR := $(realpath ../..)

CLEAN-FILES := data/bar.c data/stripes.c data/colors.c

PNG2C.bar := --bitmap bar,480x33x4 --palette bar_pal,16
PNG2C.stripes := --sprite stripes,256,4
PNG2C.colors := --palette stripes_pal,16

SOURCES := weave.c

include $(TOPDIR)/build/effect.mk
