TOPDIR := $(realpath ../..)

OBJECTS := data/jazzcat-sunglasses_at_night.o
CLEAN-FILES := data/drdos8x8.c

LIBS := libp61

PSF2C.drdos8x8 := --name drdos8x8 --type console

include $(TOPDIR)/build/effect.mk

OBJCOPY-CHIPMEM := --rename-section .data=.data_chip,alloc,load,data,contents

data/%.o: data/%.p61
	@echo "[OBJCOPY] $(DIR)$< -> $(DIR)$@"
	$(OBJCOPY) -I binary -O amiga $(OBJCOPY-CHIPMEM) $^ $@
