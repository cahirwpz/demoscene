TOPDIR := $(realpath ../..)

CLEAN-FILES := data/texture-16-1.c data/gradient.c data/gradient.png \
	       data/uvmap.c mainloop.o

PNG2C.texture-16-1 := --pixmap texture,128x128x8
PNG2C.gradient := --pixmap gradient,16x16x12

include $(TOPDIR)/build/effect.mk

data/uvmap.c: data/gen-uvmap.py
	@echo "[GEN] $@"
	$(PYTHON3) $^ uvmap > $@

data/gradient.png: $(GRADIENT) data/texture-16-1.png
	@echo "[GEN] $@"
	$(PYTHON3) $(GRADIENT) load $(word 2,$^) copy hue 0.25 save $@
