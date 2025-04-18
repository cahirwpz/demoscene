TOPDIR := $(realpath ../..)

CLEAN-FILES := data/dualtab.c

include $(TOPDIR)/build/effect.mk

data/dualtab.c: data/gen-dualtab.py data/colors-red.png
	@echo "[GEN] $(DIR)$@"
	$(PYTHON3) $^ > $@
