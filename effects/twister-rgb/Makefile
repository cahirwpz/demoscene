TOPDIR := $(realpath ../..)

SOURCES_GEN := data/twister-gradient.c
DATA_GEN := data/twister-left.ilbm \
	    data/twister-right.ilbm \
	    data/twister.ilbm
DATA := data/twister-texture.png

PNG2C.twister-gradient := --palette gradient,256

include $(TOPDIR)/build.effect.mk
