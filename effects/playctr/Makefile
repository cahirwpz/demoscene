TOPDIR := $(realpath ../..)

OBJECTS := data/JazzCat-Automatic-ctr.o data/JazzCat-Automatic-smp.o
CLEAN-FILES := data/lat2-08.c

LIBS := libctr

PSF2C.lat2-08 := --name latin2 --type console

include $(TOPDIR)/build.effect.mk

data/%.ctr data/%.smp: data/%.mod
	$(PYTHON3) data/CinterConvert.py $^ $@ $(@:.ctr=.smp)

data/%-ctr.o: data/%.ctr
	@echo "[OBJCOPY] $(DIR)$< -> $(DIR)$@"
	$(OBJCOPY) -I binary -O amiga $^ $@

data/%-smp.o: data/%.smp
	@echo "[OBJCOPY] $(DIR)$< -> $(DIR)$@"
	$(OBJCOPY) -I binary -O amiga $^ $@
