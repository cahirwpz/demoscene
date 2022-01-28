TOPDIR := $(realpath ../..)

CLEAN-FILES := \
	data/background.c \
	data/foreground.c

PNG2C.background := --bitmap background,384x384x3 --palette background_pal,8
PNG2C.foreground := --bitmap foreground,384x384x3 --palette foreground_pal,8

include $(TOPDIR)/build/effect.mk
