TOPDIR := $(realpath ../..)

DATA := data/ahxWaves data/ahxWaves.Location
OBJECTS := data/jazzcat-electric_city.o
CLEAN-FILES := data/lat2-08.c

LIBS := libahx

PSF2C.lat2-08 := --name latin2 --type console

include $(TOPDIR)/build/effect.mk

data/%.o: data/%.ahx
	@echo "[OBJCOPY] $(DIR)$< -> $(DIR)$@"
	$(OBJCOPY) -I binary -O amiga $^ $@
