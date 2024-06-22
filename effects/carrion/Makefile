TOPDIR := $(realpath ../..)

CLEAN-FILES := \
	data/carrion-conan-{pal,data}.{c,png} \
	data/carrion-metro-{pal,data}.{c,png}

PNG2C.carrion-conan-data := --bitmap carrion,320x256x2
PNG2C.carrion-conan-pal := --pixmap carrion_cols,4x256x12
PNG2C.carrion-metro-data := --bitmap carrion,320x256x2
PNG2C.carrion-metro-pal := --pixmap carrion_cols,4x256x12

include $(TOPDIR)/build/effect.mk

%-data.png %-pal.png: %.png
	@echo "[SPLITIMG] $(DIR)$< -> $(DIR)$@"
	$(PYTHON3) data/split-img.py 4 $<
