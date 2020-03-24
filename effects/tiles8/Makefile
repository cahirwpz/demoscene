TOPDIR := $(realpath ../..)

DATA := data/tiles.png data/tiles-c.png 
CLEAN-FILES := data/twist.c data/twist.png \
	       data/twist-colors.c data/twist-colors.png

PNG2C.twist := --pixmap twist,38x32x8
PNG2C.twist-colors := --pixmap colors,256x1x12

include $(TOPDIR)/build.effect.mk

data/twist.png: data/twist.py
	@echo "[GEN] $(DIR)$@"
	$(PYTHON3) $^ $@

data/twist-colors.png: data/twist-colors.py
	@echo "[GEN] $(DIR)$@"
	$(PYTHON3) $^ $@
