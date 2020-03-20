TOPDIR := $(realpath ../..)

SOURCES_GEN := data/running.asm
DATA_GEN := data/running-pal.ilbm data/running.bin

include $(TOPDIR)/build.effect.mk

%.asm: %.gif
	@echo "[ANIM] $(DIR)$< -> $(DIR)$@"
	$(PYTHON3) data/gen-anim.py $< > $@
