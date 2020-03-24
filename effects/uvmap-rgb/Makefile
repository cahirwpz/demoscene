TOPDIR := $(realpath ../..)

CLEAN-FILES := data/texture-rgb.c
DATA_GEN := data/uvmap-rgb.png

PNG2C.texture-rgb := --pixmap image,128x128x12

include $(TOPDIR)/build.effect.mk

data/uvmap-rgb.png: data/gen-uvmap.py
	@echo "[GEN] $@"
	$(PYTHON3) $< $(@:%.png=%)
