TOPDIR := $(realpath ../..)

CLEAN-FILES := data/plotter-flares.c data/plotter-flares.png

PNG2C.plotter-flares := --bitmap flares,16x128x3 --palette flares_pal,8

include $(TOPDIR)/build/effect.mk

data/plotter-flares.png:
	@echo "[GEN] $(DIR)$@"
	$(PYTHON3) data/plotter-flares.py $@
