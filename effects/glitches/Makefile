TOPDIR := $(realpath ../..)

CLEAN-FILES := data/dragon-bg.c data/speccy.c data/tearing-table.c

PNG2C.dragon-bg := --bitmap background,320x256x4 --palette background,16
PNG2C.speccy := --palette speccy,16

data/tearing-table.c:
	$(PYTHON3) data/gen_tearing.py  $< $@ || (rm -f $@ && exit 1)

include $(TOPDIR)/build/effect.mk
