TOPDIR := $(realpath ../..)

SOURCES_GEN := data/wireframe-pal.c
DATA := data/pilka.3d

LIBS := lib3d

include $(TOPDIR)/build.effect.mk

data/wireframe-pal.c: PNG2C += --palette wireframe_pal,16
