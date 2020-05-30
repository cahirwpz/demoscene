TOPDIR := $(realpath ../..)

CLEAN-FILES := data/face.c

include $(TOPDIR)/build/effect.mk

%.c: %-pchg.png
	@echo "[PCHG2C] $(DIR)$< -> $(DIR)$@"
	$(PCHG2C) $< > $@
