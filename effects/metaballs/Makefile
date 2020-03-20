TOPDIR := $(realpath ../..)

DATA_GEN := data/metaball.png \
	    data/metaball.ilbm \
	    data/metaball-bg-left-1.ilbm \
	    data/metaball-bg-right-1.ilbm

data/metaball.png: data/gen-metaball.py
	@echo "[GEN] $(DIR)$@"
	$(PYTHON3) $< $@

include $(TOPDIR)/build.effect.mk
