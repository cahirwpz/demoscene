TOPDIR := $(realpath ../..)

SOURCES_GEN := data/shapes-pal.c
DATA := data/night.2d data/boxes.2d

LIBS := lib2d

include $(TOPDIR)/build.effect.mk

data/shapes-pal.c: PNG2C += --palette shapes_pal,16
