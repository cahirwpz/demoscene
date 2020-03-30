TOPDIR := $(realpath ../..)

OBJECTS := data/jazzcat-sunglasses_at_night.o

LIBS := libp61

include $(TOPDIR)/build.effect.mk

OBJCOPY-CHIPMEM := --rename-section .data=.data_chip,alloc,load,data,contents

data/%.o: data/%.p61
	@echo "[OBJCOPY] $(DIR)$< -> $(DIR)$@"
	$(OBJCOPY) -I binary -O amiga $(OBJCOPY-CHIPMEM) $^ $@
