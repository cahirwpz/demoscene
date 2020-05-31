TOPDIR := $(realpath ../..)

CLEAN-FILES := data/layer0.c data/layer1-map.c data/layer1-tiles.c \
	       data/layer2-map.c data/layer2-tiles.c data/gradient.c

PNG2C.layer0 := --bitmap layer0,320x176x1
PNG2C.layer1-tiles := --bitmap layer1_tiles,16x2960x2 --palette layer1_pal,4
PNG2C.layer2-tiles := --bitmap layer2_tiles,16x4128x3 --palette layer2_pal,8
PNG2C.gradient := --pixmap gradient,1x176x12

include $(TOPDIR)/build/effect.mk

%-tiles.png: %.tmx
	@echo "[TMX] $(DIR)$< -> $(DIR)$@"
	$(TMXCONV) -tiles $@ $<

%-map.c: %.tmx
	@echo "[TMX] $(DIR)$< -> $(DIR)$@"
	$(TMXCONV) -source $@ $<
