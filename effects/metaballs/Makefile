TOPDIR := $(realpath ../..)

SOURCES_GEN := data/metaball.c \
	       data/metaball-bg-left-1.c \
	       data/metaball-bg-right-1.c

PNG2C.metaball-bg-left-1 := --bitmap bgLeft,32x256x5 --palette metaball_pal,32
PNG2C.metaball-bg-right-1 := --bitmap bgRight,32x256x5
PNG2C.metaball := --bitmap metaball,80x80x5

include $(TOPDIR)/build.effect.mk

data/metaball.png: data/gen-metaball.py
	@echo "[GEN] $(DIR)$@"
	$(PYTHON3) $< $@
