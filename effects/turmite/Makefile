TOPDIR := $(realpath ../..)

CLEAN-FILES := \
	data/turmite-pal.c \
	data/scene.c 

PNG2C.turmite-pal := --palette turmite_pal,16
PNG2C.scene := --bitmap scene,256x256x1

SOURCES := turmite.c

include $(TOPDIR)/build/effect.mk
