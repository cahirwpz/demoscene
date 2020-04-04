TOPDIR := $(realpath ../..)

OBJECTS := data/JazzCat-Automatic-ctr.o data/JazzCat-Automatic-smp.o

LIBS := libctr

include $(TOPDIR)/build.effect.mk

data/%.ctr data/%.smp: data/%.mod
	$(PYTHON3) data/CinterConvert.py $^ $@ $(@:.ctr=.smp)

data/%-ctr.o: data/%.ctr
	@echo "[OBJCOPY] $(DIR)$< -> $(DIR)$@"
	$(OBJCOPY) -I binary -O amiga $^ $@

data/%-smp.o: data/%.smp
	@echo "[OBJCOPY] $(DIR)$< -> $(DIR)$@"
	$(OBJCOPY) -I binary -O amiga $^ $@
