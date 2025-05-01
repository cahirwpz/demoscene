TOPDIR := $(realpath ../..)

CLEAN-FILES := data/plotter-flares.c data/plotter-flares.png

PNG2C.plotter-flares := --bitmap flares,16x128x3 --palette flares,8
PNG2C.background-1 := --sprite background_1,256,8 --palette background,4
PNG2C.background-2 := --sprite background_2,256,8
PNG2C.background-3 := --sprite background_3,256,8
PNG2C.gradient := --pixmap background_cols,4x256x12,+onlydata

include $(TOPDIR)/build/effect.mk

data/plotter-flares.png:
	@echo "[GEN] $(DIR)$@"
	$(PYTHON3) data/plotter-flares.py $@
