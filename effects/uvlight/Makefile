TOPDIR := $(realpath ../..)

DATA := data/texture.png data/torus-light.png
DATA_GEN := data/torus-map.png

include $(TOPDIR)/build.effect.mk

data/torus-map.png: data/gen-uvmap.py
	@echo "[GEN] $@"
	$(PYTHON3) $< $(@:%-map.png=%)
