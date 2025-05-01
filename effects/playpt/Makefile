TOPDIR := $(realpath ../..)

MODULE := virgill-amigahub
OBJECTS := data/$(MODULE).trk.o
CLEAN-FILES := data/lat2-08.c

AKLANG := 1
DELTA := 0

ifeq ($(AKLANG),1)
# XXX: Always consult this value with AmigaKlang output,
# otherwise you'll experience a really nasty debugging session. */
CPPFLAGS += -DAKLANG_BUFLEN=36864
CPPFLAGS += -DAKLANG
OBJECTS += data/$(MODULE).o
else
OBJECTS += data/$(MODULE).smp.o
endif

ifeq ($(DELTA),1)
CPPFLAGS += -DDELTA
PTSPLIT.$(MODULE) := --delta
endif

PSF2C.lat2-08 := --name latin2 --type console

LIBS := libpt

include $(TOPDIR)/build/effect.mk

CLEAN-FILES += data/$(MODULE).smp data/$(MODULE).trk

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
