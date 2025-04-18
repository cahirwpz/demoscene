TOPDIR := $(realpath ../..)

CLEAN-FILES := data/text-scroll-font.c data/background.c
OBJECTS := data/text-scroll.o

PNG2C.text-scroll-font := --bitmap font,768x8x1,+cpuonly --palette font,8,+store_unused
PNG2C.background := --bitmap background,640x256x1,+onlydata --palette background,2

include $(TOPDIR)/build/effect.mk

data/%.o: data/%.txt
	@echo "[OBJCOPY] $(DIR)$< -> $(DIR)$@"
	$(OBJCOPY) -I binary -O amiga \
	  --redefine-sym _binary_data_$(subst -,_,$*)_txt_start=_Text \
	  --redefine-sym _binary_data_$(subst -,_,$*)_txt_end=_TextEnd \
	  --redefine-sym _binary_data_$(subst -,_,$*)_txt_size=_TextSize \
	  $^ $@

