TOPDIR := $(realpath ../..)

SOURCES_GEN := data/flatshade-pal.c
DATA_GEN := data/codi.3d data/pilka.3d

LIBS := lib3d

include $(TOPDIR)/build.effect.mk

data/flatshade-pal.c: PNG2C += --palette flatshade_pal,16
