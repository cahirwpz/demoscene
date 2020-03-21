TOPDIR := $(realpath ../..)

SOURCES_GEN := data/twister-gradient.c
DATA_GEN := data/twister-left.ilbm \
	    data/twister-right.ilbm \
	    data/twister.ilbm
DATA := data/twister-texture.png

include $(TOPDIR)/build.effect.mk

data/twister-gradient.c: PNG2C += --palette gradient,256
