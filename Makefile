TOPDIR = $(realpath .)

SUBDIRS = tools lib effects
SUBDIRS = tools lib system effects
EXTRA-FILES = tags cscope.out
CLEAN-FILES = bootloader.bin system-api.lds

all: a500rom.bin bootloader.bin amiga.lds build

include $(TOPDIR)/build/common.mk

a500rom.bin: ASFLAGS += -phxass
bootloader.bin: ASFLAGS += -phxass

%.lds: system-api.py %.in 
	python3 $^ $@

amiga.lds: system-api.lds
	touch $@

FILES := $(shell find include lib system -type f -iname '*.[ch]')

tags:
	ctags -R $(FILES)

cscope.out:
	cscope -b $(FILES)
