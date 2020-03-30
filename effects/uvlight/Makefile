TOPDIR := $(realpath ../..)

CLEAN-FILES := data/torus-map.c data/torus-light.c data/texture.c

PNG2C.torus-light := --pixmap light,80x64x8
PNG2C.texture := --pixmap texture_raw,128x128x8 --palette texture_pal,224

include $(TOPDIR)/build.effect.mk

data/torus-map.c: data/gen-uvmap.py
	@echo "[GEN] $@"
	$(PYTHON3) $< data/torus uvmap > $@
