TOPDIR := $(realpath ../..)

CLEAN-FILES := data/skulls_map.c data/light.c data/skulls_128col.c

PNG2C.skulls_128col := --pixmap skulls,160x128x8 --palette skulls,128

include $(TOPDIR)/build/effect.mk

data/skulls_map.c: data/bumpmap.py data/skulls_map.png
	@echo "[GEN] $@"
	$(PYTHON3) $^ bumpmap > $@

data/light.c: data/light.py
	@echo "[GEN] $@"
	$(PYTHON3) $^ light > $@
