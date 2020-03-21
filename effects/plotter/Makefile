TOPDIR := $(realpath ../..)

SOURCES_GEN := data/plotter-flares.c

include $(TOPDIR)/build.effect.mk

data/plotter-flares.png:
	@echo "[GEN] $(DIR)$@"
	$(PYTHON3) data/plotter-flares.py $@

data/plotter-flares.c: PNG2C += --bitmap flares,16x128x3 --palette flares_pal,8
