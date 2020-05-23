TOPDIR := $(realpath ..)

SOURCES := \
	bootloader.s \
	crt0.s

SUBDIRS := \
	libc \
	lib2d \
	lib3d \
	libahx \
	libblit \
	libctr \
	libgui \
	libgfx \
	libp61 \
	libsync \
	libsys

BUILD-FILES := crt0.o bootloader.bin tags cscope.out

include $(TOPDIR)/build.mk

bootloader.bin: ASFLAGS += -phxass

FILES := $(shell find $(SUBDIRS) include -iname '*.c' -or -iname '*.h')

tags:
	ctags -R $(FILES)

cscope.out:
	cscope -b $(FILES)
