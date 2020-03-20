TOPDIR := $(realpath ../..)

DATA_GEN := data/light.png data/bumpmap.bin
DATA := data/dragon.png

include $(TOPDIR)/build.effect.mk

data/bumpmap.bin: data/bumpmap.py data/bumpmap.png
	@echo "[GEN] $@"
	$(PYTHON3) data/bumpmap.py data/bumpmap.png

data/light.png: data/light.py
	@echo "[GEN] $@"
	$(PYTHON3) data/light.py $@
