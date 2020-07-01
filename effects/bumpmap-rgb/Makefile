TOPDIR := $(realpath ../..)

CLEAN-FILES := data/bumpmap.c data/light.c data/dragon.c

PNG2C.dragon := --pixmap dragon,80x64x8 --palette dragon_pal,97

include $(TOPDIR)/build/effect.mk

data/bumpmap.c: data/bumpmap.py data/bumpmap.png
	@echo "[GEN] $@"
	$(PYTHON3) $^ bumpmap > $@

data/light.c: data/light.py
	@echo "[GEN] $@"
	$(PYTHON3) $^ light > $@
