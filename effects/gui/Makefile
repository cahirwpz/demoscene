TOPDIR := $(realpath ../..)

DATA :=	data/koi8r.8x8.font
DATA_GEN := data/koi8r.8x8.png data/koi8r.8x8.ilbm
CLEAN-FILES := data/toggle_0.c data/toggle_1.c

LIBS := libgui

PNG2C.toggle_0 := --bitmap toggle_0,16x16x1
PNG2C.toggle_1 := --bitmap toggle_1,16x16x1

include $(TOPDIR)/build.effect.mk
