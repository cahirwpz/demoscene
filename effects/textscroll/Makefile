TOPDIR := $(realpath ../..)

CLEAN-FILES := data/text-scroll-font.c
OBJECTS := data/text-scroll.o

PNG2C.text-scroll-font := --bitmap font,768x8x1 --palette font_pal,2

include $(TOPDIR)/build/effect.mk

data/%.o: data/%.txt
	@echo "[OBJCOPY] $(DIR)$< -> $(DIR)$@"
	$(OBJCOPY) -I binary -O amiga $^ $@
