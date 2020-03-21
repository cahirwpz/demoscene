TOPDIR := $(realpath ../..)

CLEAN-FILES := data/twister-gradient.c data/twister.c
DATA_GEN := data/twister-left.ilbm data/twister-right.ilbm
DATA := data/twister-texture.png

PNG2C.twister := --bitmap twister,144x256x5
PNG2C.twister-gradient := --palette gradient,256

include $(TOPDIR)/build.effect.mk
