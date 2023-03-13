TOPDIR := $(realpath ../..)

OBJECTS := data/virgill-amigahub-trk.o data/virgill-amigahub.o
CLEAN-FILES := data/lat2-08.c data/*.smp data/*.trk

LIBS := libpt

PSF2C.lat2-08 := --name latin2 --type console

include $(TOPDIR)/build/effect.mk

data/%.mod.trk data/%.mod.smp: data/%.mod
	@echo "[PTSPLIT] $(DIR)$< -> $(DIR)$@"
	$(PYTHON) data/ptsplit.py $<

data/%-trk.o: data/%.mod.trk
	@echo "[OBJCOPY] $(DIR)$< -> $(DIR)$@"
	$(OBJCOPY) -I binary -O amiga \
	  --redefine-sym _binary_data_$(subst -,_,$*)_mod_trk_start=_Module \
	  --redefine-sym _binary_data_$(subst -,_,$*)_mod_trk_end=_ModuleEnd \
	  --redefine-sym _binary_data_$(subst -,_,$*)_mod_trk_size=_ModuleSize \
	  $^ $@

data/%-smp.o: data/%.mod.smp
	@echo "[OBJCOPY] $(DIR)$< -> $(DIR)$@"
	$(OBJCOPY) -I binary -O amiga \
	  --redefine-sym _binary_data_$(subst -,_,$*)_mod_smp_start=_Samples \
	  --redefine-sym _binary_data_$(subst -,_,$*)_mod_smp_end=_SamplesEnd \
	  --redefine-sym _binary_data_$(subst -,_,$*)_mod_smp_size=_SamplesSize \
	  --rename-section .data=.datachip,alloc,load,data,contents $^ $@
