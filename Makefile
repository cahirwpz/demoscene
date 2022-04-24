TOPDIR = $(realpath .)

SUBDIRS = tools lib effects
SUBDIRS = tools lib system effects
EXTRA-FILES = tags cscope.out
CLEAN-FILES = bootloader.bin 

all: bootloader.bin build

include $(TOPDIR)/build/common.mk

bootloader.bin: ASFLAGS += -phxass

FILES := $(shell find include lib system -type f -iname '*.[ch]')

tags:
	ctags -R $(FILES)

cscope.out:
	cscope -b $(FILES)
