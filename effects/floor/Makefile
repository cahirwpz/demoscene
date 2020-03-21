TOPDIR := $(realpath ../..)

SOURCES_GEN := data/stripes.c
DATA_GEN := data/floor.ilbm

include $(TOPDIR)/build.effect.mk

data/stripes.c: PNG2C += --palette stripes_pal,16
