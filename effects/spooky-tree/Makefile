TOPDIR := $(realpath ../..)

CLEAN-FILES := data/tree-pal.* data/tree-data.* \
	data/ghost64x_01.c data/ghost64x_02.c data/ghost64x_03.c \
	data/ghost32x_01.c data/ghost32x_02.c data/ghost32x_03.c \

PNG2C.tree-data := --bitmap tree,320x1100x3
PNG2C.tree-pal := --pixmap tree_cols,8x1100x12
PNG2C.ghost64x_01 := --sprite ghost1,80,4 --palette ghost,4
PNG2C.ghost64x_02 := --sprite ghost2,80,4
PNG2C.ghost64x_03 := --sprite ghost3,80,4
PNG2C.ghost32x_01 := --sprite smallGhost1,32,2 --palette smallGhost,4
PNG2C.ghost32x_02 := --sprite smallGhost2,32,2
PNG2C.ghost32x_03 := --sprite smallGhost3,32,2

include $(TOPDIR)/build/effect.mk

%-data.png %-pal.png: %.png
	@echo "[SPLITIMG] $(DIR)$< -> $(DIR)$@"
	$(PYTHON3) data/split-img.py --force 8 $<
