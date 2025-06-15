TOPDIR := $(realpath ../..)

MODULE := JazzCat-XenoCoq
OBJECTS := data/$(MODULE).trk.o data/$(MODULE).smp.o

DELTA := 1

ifeq ($(DELTA),1)
CPPFLAGS += -DDELTA
PTSPLIT.$(MODULE) := --delta
endif

LIBS := libpt

include $(TOPDIR)/build/effect.mk

CLEAN-FILES += data/$(MODULE)*.o data/$(MODULE).smp data/$(MODULE).trk

OBJCOPY-CHIP := --rename-section .data=.datachip,alloc,load,data,contents

data/%.trk.o: data/%.trk
	@echo "[OBJCOPY] $(DIR)$< -> $(DIR)$@"
	$(OBJCOPY) -I binary -O amiga \
	  --redefine-sym _binary_data_$(subst -,_,$*)_trk_start=_Module \
	  --redefine-sym _binary_data_$(subst -,_,$*)_trk_end=_ModuleEnd \
	  --redefine-sym _binary_data_$(subst -,_,$*)_trk_size=_ModuleSize \
	  $^ $@

data/%.smp.o: data/%.smp
	@echo "[OBJCOPY] $(DIR)$< -> $(DIR)$@"
	$(OBJCOPY) $(OBJCOPY-CHIP) -I binary -O amiga \
	  --redefine-sym _binary_data_$(subst -,_,$*)_smp_start=_Samples \
	  --redefine-sym _binary_data_$(subst -,_,$*)_smp_end=_SamplesEnd \
	  --redefine-sym _binary_data_$(subst -,_,$*)_smp_size=_SamplesSize \
	  $^ $@
