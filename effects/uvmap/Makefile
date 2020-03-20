TOPDIR := $(realpath ../..)

DATA := data/texture-16-1.png data/texture-16-2.png
DATA_GEN := data/uvmap.png data/gradient.png

include $(TOPDIR)/build.effect.mk

data/uvmap.png: data/gen-uvmap.py
	@echo "[GEN] $@"
	$(PYTHON3) $^ $(@:%.png=%)

data/gradient.png: data/gradient.py data/texture-16-1.png data/texture-16-2.png
	@echo "[GEN] $@"
	$(PYTHON3) $^ $@

