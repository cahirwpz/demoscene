TOPDIR := $(realpath ../..)

CLEAN-FILES := data/dragon-bg.c data/texture-15.c data/ball.c

PNG2C.dragon-bg := --bitmap background,320x256x4 --palette background,16
PNG2C.texture-15 := --pixmap texture,128x128x4 --palette texture,16

include $(TOPDIR)/build/effect.mk

data/ball.c: data/gen-ball.py
	@echo "[GEN] $@"
	$(PYTHON3) $< uvmap > $@
