TOPDIR := $(realpath ../..)

CLEAN-FILES := data/dancing.c

include $(TOPDIR)/build/effect.mk

%.c: %.csv
	@echo "[ANIM] $(DIR)$< -> $(DIR)$@"
	$(PYTHON3) encode.py $< > $@
