TOPDIR := $(realpath ../..)

DATA := data/texture-rgb.png
DATA_GEN := data/uvmap-rgb.png

include $(TOPDIR)/build.effect.mk

data/uvmap-rgb.png: data/gen-uvmap.py
	@echo "[GEN] $@"
	$(PYTHON3) $< $(@:%.png=%)
