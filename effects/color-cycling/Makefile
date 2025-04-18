TOPDIR := $(realpath ../..)

CLEAN-FILES := data/image.c data/image-cycling.c

PNG2C.image := --bitmap logo,320x256x5 --palette logo,32
DRNG2C.image := -name logo

include $(TOPDIR)/build/effect.mk

data/%-cycling.c: data/%.iff
	@echo "[DRNG2C] $(DIR)$< -> $(DIR)$@"
	$(DRNG2C) $(DRNG2C.$*) -o $@ $<
