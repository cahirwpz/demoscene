TOPDIR := $(realpath ../..)

SOURCES_GEN := data/running.asm data/running-pal.c
DATA_GEN := data/running.bin

include $(TOPDIR)/build.effect.mk

%.asm: %.gif
	@echo "[ANIM] $(DIR)$< -> $(DIR)$@"
	$(PYTHON3) data/gen-anim.py $< > $@

data/running-pal.c: PNG2C += --palette running_pal,16
