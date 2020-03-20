TOPDIR := $(realpath ../..)

DATA_GEN := data/plotter-flares.png \
	    data/plotter-flares.ilbm

include $(TOPDIR)/build.effect.mk

data/plotter-flares.png:
	@echo "[GEN] $(DIR)$@"
	$(PYTHON3) data/plotter-flares.py $@
	$(OPTIPNG) $@
