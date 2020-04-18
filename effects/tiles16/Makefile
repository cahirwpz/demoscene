TOPDIR := $(realpath ../..)

CLEAN-FILES := data/MagicLand-tiles.c data/MagicLand-map.c \
	       data/MagicLand-tiles.png

PNG2C.MagicLand-tiles := --bitmap tiles,16x13360x5,+interleaved \
	                 --palette tiles_pal,17

include $(TOPDIR)/build.effect.mk

%-tiles.png: %.tmx
	@echo "[TMX] $(DIR)$< -> $(DIR)$@"
	$(TMXCONV) -tiles $@ $<

%-map.c: %.tmx
	@echo "[TMX] $(DIR)$< -> $(DIR)$@"
	$(TMXCONV) -source $@ $<
