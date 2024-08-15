TOPDIR := $(realpath ../..)

CLEAN-FILES := data/dancing.c data/cock.c data/dancing-pal.c 

PNG2C.dancing-pal := --pixmap gradient,16x22x12

include $(TOPDIR)/build/effect.mk

data/%.c: data/%.svg
	@echo "[ANIM] $(DIR)$< -> $(DIR)$@"
	$(PYTHON3) encode.py $< > $@
