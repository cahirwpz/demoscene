TOPDIR := $(realpath ../..)

CLEAN-FILES := data/twister-gradient.c data/twister.c \
	       data/twister-left.c data/twister-right.c \
	       data/twister-texture.c

PNG2C.twister := --bitmap twister,144x256x5
PNG2C.twister-gradient := --palette gradient,256
PNG2C.twister-left := --sprite left,256,2
PNG2C.twister-right := --sprite right,256,2
PNG2C.twister-texture := --pixmap texture,31x85x12

include $(TOPDIR)/build/effect.mk
