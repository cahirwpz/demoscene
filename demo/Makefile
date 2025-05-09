TOPDIR := $(realpath ..)

DEMONAME = Demo
MODULE := JazzCat-ElectricLifeforms

OBJECTS := data/$(MODULE).trk.o data/$(MODULE).smp.o

DELTA := 1

ifeq ($(DELTA),1)
CPPFLAGS += -DDELTA
PTSPLIT.$(MODULE) := --delta
endif

LDSCRIPT := demo.lds

LIBS := libpt

CLEAN-FILES := data/$(MODULE).smp data/$(MODULE).trk data/loader.c

SOURCES := main.c load.c empty.c
LOADABLES := spooky-town.exe
MAIN := #

CPPFLAGS += -DDELTA=$(DELTA) -I.
DEMO := 1
export DEMO

PNG2C.loader := --bitmap loader,64x64x1 --palette loader,2

include $(TOPDIR)/build/effect.mk

$(LOADABLES): demo.exe
	@echo "[SPLITEXE] $^ -> $(LOADABLES)"
	$(SPLITEXE) $^ $(LOADABLES)

demo.lds: demo.json
	@echo "[LDS] $< -> $@"
	$(PYTHON3) gen-lds.py $< $@

CLEAN-FILES += demo.lds

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

data/%.o: data/%.txt
	@echo "[OBJCOPY] $(DIR)$< -> $(DIR)$@"
	$(OBJCOPY) -I binary -O amiga \
	  --redefine-sym _binary_data_$(subst -,_,$*)_txt_start=_Text \
	  --redefine-sym _binary_data_$(subst -,_,$*)_txt_end=_TextEnd \
	  --redefine-sym _binary_data_$(subst -,_,$*)_txt_size=_TextSize \
	  $^ $@

REPOHEAD := $(shell git rev-parse --short HEAD)

%.exe.packed: %.exe
	@echo "[PACK] $(DIR)$< -> $(DIR)$@"
	Shrinkler -o -f 0xdff180 \
	  -t "$(DEMONAME) by Ghostown (build: $(REPOHEAD))" $< $@
