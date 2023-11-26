TOPDIR := $(realpath ../..)

CLEAN-FILES := data/running.c data/running-pal.c

PNG2C.running-pal := --palette running,16

include $(TOPDIR)/build/effect.mk

%.c: %.gif
	@echo "[ANIM] $(DIR)$< -> $(DIR)$@"
	$(PYTHON3) data/gen-anim.py $< > $@
