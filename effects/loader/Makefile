TOPDIR := $(realpath ../..)

CLEAN-FILES := data/loader.c

PNG2C.loader := --bitmap loader,320x256x3 --palette loader,2

MODULE := JazzCat-CockbustersLoader
OBJECTS := data/$(MODULE).trk.o data/$(MODULE).smp.o

LIBS := libpt

include $(TOPDIR)/build/effect.mk

CLEAN-FILES += data/$(MODULE)*.o data/$(MODULE).smp data/$(MODULE).trk

data/%.trk data/%.smp: data/%.mod
	$(PYTHON3) $(TOPDIR)/effects/playpt/data/ptsplit.py $^
	mv data/$*.mod.trk data/$*.trk
	mv data/$*.mod.smp data/$*.smp

OBJCOPY-CHIP := --rename-section .data=.datachip,alloc,load,data,contents

data/%.trk.o: data/%.trk
	@echo "[OBJCOPY] $(DIR)$< -> $(DIR)$@"
	$(OBJCOPY) -I binary -O amiga \
	  --redefine-sym _binary_data_$(subst -,_,$*)_trk_start=_LoaderModule \
	  --redefine-sym _binary_data_$(subst -,_,$*)_trk_end=_LoaderModuleEnd \
	  --redefine-sym _binary_data_$(subst -,_,$*)_trk_size=_LoaderModuleSize \
	  $^ $@

data/%.smp.o: data/%.smp
	@echo "[OBJCOPY] $(DIR)$< -> $(DIR)$@"
	$(OBJCOPY) $(OBJCOPY-CHIP) -I binary -O amiga \
	  --redefine-sym _binary_data_$(subst -,_,$*)_smp_start=_LoaderSamples \
	  --redefine-sym _binary_data_$(subst -,_,$*)_smp_end=_LoaderSamplesEnd \
	  --redefine-sym _binary_data_$(subst -,_,$*)_smp_size=_LoaderSamplesSize \
	  $^ $@

