TOPDIR := $(realpath ../..)

CLEAN-FILES := data/layer0.c data/layer[12]-map.c data/layer[12]-tiles.c \
	       data/layer[12].tmx data/layer[12]_report.png \
	       data/layer[12]_tiles.png data/gradient.c

PNG2C.layer0 := --bitmap layer0,320x176x1
PNG2C.layer1-tiles := --bitmap layer1_tiles,16x2960x2 --palette layer1_pal,4
PNG2C.layer2-tiles := --bitmap layer2_tiles,16x4128x3 --palette layer2_pal,8
PNG2C.gradient := --pixmap gradient,1x176x12

include $(TOPDIR)/build/effect.mk

%.tmx: %.png
	@echo "[MAKETMX] $(DIR)$< -> $(DIR)$@"
	$(MAKETMX) $^

%-tiles.png: %.tmx
	@echo "[TMXCONV] $(DIR)$< -> $(DIR)$@"
	$(TMXCONV) -tiles $@ $<

%-map.c: %.tmx
	@echo "[TMXCONV] $(DIR)$< -> $(DIR)$@"
	$(TMXCONV) -source $@ $<
