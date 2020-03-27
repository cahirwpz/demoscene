TOPDIR := $(realpath ../..)

DATA_GEN := data/MagicLand-tiles.ilbm
CLEAN-FILES := data/MagicLand.c data/MagicLand-tiles.png

include $(TOPDIR)/build.effect.mk

%-tiles.png: %.tmx
	@echo "[TMX] $(DIR)$< -> $(DIR)$@"
	$(TMXCONV) tiles $< > $@

%.c: %.tmx
	@echo "[TMX] $(DIR)$< -> $(DIR)$@"
	$(TMXCONV) source $< > $@
