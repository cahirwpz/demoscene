TOPDIR := $(realpath ../..)

CLEAN-FILES := data/texture-16-1.c data/gradient.c data/gradient.png \
	       data/uvmap.c

PNG2C.texture-16-1 := --pixmap texture,128x128x8 --palette texture_pal,16
PNG2C.gradient := --pixmap gradient,16x16x12

include $(TOPDIR)/build.effect.mk

data/uvmap.c: data/gen-uvmap.py
	@echo "[GEN] $@"
	$(PYTHON3) $^ uvmap > $@

data/gradient.png: data/gradient.py data/texture-16-1.png data/texture-16-2.png
	@echo "[GEN] $@"
	$(PYTHON3) $^ $@
