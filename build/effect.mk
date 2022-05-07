EFFECT := $(notdir $(CURDIR))

ifndef SOURCES
SOURCES = $(EFFECT).c
endif

LIBS += libblit libgfx libmisc libc
LDEXTRA = $(TOPDIR)/system/system.a
LDEXTRA += $(foreach lib,$(LIBS),$(TOPDIR)/lib/$(lib)/$(lib).a)

CRT0 = $(TOPDIR)/system/crt0.o
BOOTLOADER = $(TOPDIR)/bootloader.bin
ROMSTARTUP = $(TOPDIR)/a500rom.bin

EXTRA-FILES += $(DATA_GEN) $(EFFECT).img $(EFFECT).adf $(EFFECT).rom
CLEAN-FILES += $(DATA_GEN) $(EFFECT).exe $(EFFECT).exe.dbg $(EFFECT).exe.map 

all: build

# Check if library is up-to date if someone is asking explicitely
$(TOPDIR)/lib/lib%.a: FORCE
	$(MAKE) -C $(dir $@) $(notdir $@)

$(TOPDIR)/system/%.o: FORCE
	$(MAKE) -C $(dir $@) $(notdir $@)

$(TOPDIR)/system/%.a: FORCE
	$(MAKE) -C $(dir $@) $(notdir $@)

$(TOPDIR)/effects/%.a: FORCE
	$(MAKE) -C $(dir $@) $(notdir $@)

$(TOPDIR)/%.bin: FORCE
	$(MAKE) -C $(dir $@) $(notdir $@)

include $(TOPDIR)/build/common.mk

$(EFFECT).exe.dbg $(EFFECT).exe: $(CRT0) $(OBJECTS) $(LDEXTRA)
	@echo "[LD] $(addprefix $(DIR),$(OBJECTS)) -> $(DIR)$@"
	$(LD) $(LDFLAGS) -T$(TOPDIR)/amiga.lds -Map=$@.map -o $@ \
		--start-group $^ --end-group
	$(CP) $@ $@.dbg
	$(STRIP) $@

data/%.c: data/%.lwo
	@echo "[LWO] $(DIR)$< -> $(DIR)$@"
	$(LWO2C) $(LWO2C.$*) -f $< $@

data/%.c: data/%.psfu
	@echo "[PSF] $(DIR)$^ -> $(DIR)$@"
	$(PSF2C) $(PSF2C.$*) $< > $@ || (rm -f $@ && exit 1)

data/%.c: data/%.png
	@echo "[PNG] $(DIR)$< -> $(DIR)$@"
	$(PNG2C) $(PNG2C.$*) $< > $@ || (rm -f $@ && exit 1)

data/%.c: data/%.2d
	@echo "[2D] $(DIR)$< -> $(DIR)$@"
	$(CONV2D) $(CONV2D.$*) $< > $@ || (rm -f $@ && exit 1)

data/%.c: data/%.sync
	@echo "[SYNC] $(DIR)$< -> $(DIR)$@"
	$(SYNC2C) $(SYNC2C.$*) $< > $@ || (rm -f $@ && exit 1)

%.img: %.exe $(DATA) $(DATA_GEN)
	@echo "[IMG] $(addprefix $(DIR),$*.exe $(DATA) $(DATA_GEN)) -> $(DIR)$@"
	$(FSUTIL) create $@ $(filter-out %bootloader.bin,$^)

%.adf: %.img $(BOOTLOADER) 
	@echo "[ADF] $(DIR)$< -> $(DIR)$@"
	$(ADFUTIL) -b $(BOOTLOADER) $< $@ 

%.rom: %.img $(ROMSTARTUP)
	@echo "[ROM] $(DIR)$< -> $(DIR)$@"
	$(ROMUTIL) $(ROMSTARTUP) $< $@ 

# Default debugger - can be changed by passing DEBUGGER=xyz to make.
DEBUGGER ?= gdb

run-floppy: $(EFFECT).exe.dbg $(EFFECT).adf
	$(LAUNCH) -e $(EFFECT).exe.dbg -f $(EFFECT).adf

debug-floppy: $(EFFECT).exe.dbg $(EFFECT).adf
	$(LAUNCH) -d $(DEBUGGER) -f $(EFFECT).adf -e $(EFFECT).exe.dbg

run: $(EFFECT).rom $(EFFECT).exe.dbg $(EFFECT).adf
	$(LAUNCH) -r $(EFFECT).rom -e $(EFFECT).exe.dbg -f $(EFFECT).adf

debug: $(EFFECT).rom $(EFFECT).exe.dbg $(EFFECT).adf
	$(LAUNCH) -d $(DEBUGGER) -r $(EFFECT).rom -e $(EFFECT).exe.dbg -f $(EFFECT).adf

.PHONY: run debug run-floppy debug-floppy
.PRECIOUS: $(BOOTLOADER) $(EFFECT).img
