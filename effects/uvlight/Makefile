TOPDIR := $(realpath ../..)

DATA := data/texture.png data/torus-light.png
CLEAN-FILES := data/torus-map.c

include $(TOPDIR)/build.effect.mk

data/torus-map.c: data/gen-uvmap.py
	@echo "[GEN] $@"
	$(PYTHON3) $< data/torus uvmap > $@
