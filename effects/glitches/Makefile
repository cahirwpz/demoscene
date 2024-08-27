TOPDIR := $(realpath ../..)

CLEAN-FILES := data/dragon-bg.c data/speccy.c data/tearing-table.c

PNG2C.dragon-bg := --bitmap background,320x512x4 --palette background,16
PNG2C.speccy := --palette speccy,16

include $(TOPDIR)/build/effect.mk

data/tearing-table.c:
	$(PYTHON3) data/gen_tearing.py  $< $@ || (rm -f $@ && exit 1)
