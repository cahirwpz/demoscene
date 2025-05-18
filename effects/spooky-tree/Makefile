TOPDIR := $(realpath ../..)

CLEAN-FILES := data/tree-pal.* data/tree-data.*

PNG2C.tree-data := --bitmap tree,320x1100x3
PNG2C.tree-pal := --pixmap tree_cols,8x1100x12

include $(TOPDIR)/build/effect.mk

%-data.png %-pal.png: %.png
	@echo "[SPLITIMG] $(DIR)$< -> $(DIR)$@"
	$(PYTHON3) data/split-img.py --force 8 $<
