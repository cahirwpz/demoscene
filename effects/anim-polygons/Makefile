TOPDIR := $(realpath ../..)

CLEAN-FILES := data/dancing.c data/dancing-pal.c 

PNG2C.dancing-pal := --pixmap gradient,16x22x12

include $(TOPDIR)/build/effect.mk

%.c: %.svg
	@echo "[ANIM] $(DIR)$< -> $(DIR)$@"
	$(PYTHON3) encode.py $< > $@
