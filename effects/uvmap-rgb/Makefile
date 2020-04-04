TOPDIR := $(realpath ../..)

CLEAN-FILES := data/texture-rgb.c data/uvmap-rgb.c

PNG2C.texture-rgb := --pixmap image,128x128x12

include $(TOPDIR)/build.effect.mk

data/uvmap-rgb.c: data/gen-uvmap.py
	@echo "[GEN] $@"
	$(PYTHON3) $< uvmap > $@
