EFFECT := $(notdir $(CURDIR))

ifndef SOURCES
SOURCES = $(EFFECT).c
endif

LOADABLES ?= $(EFFECT).exe

LIBS += libblit libgfx libmisc libc
LDEXTRA = $(TOPDIR)/system/system.a
LDEXTRA += $(foreach lib,$(LIBS),$(TOPDIR)/lib/$(lib)/$(lib).a)

CRT0 = $(TOPDIR)/system/crt0.o
MAIN ?= $(TOPDIR)/effects/main.o
BOOTLOADER = $(TOPDIR)/bootloader.bin
ROMSTARTUP = $(TOPDIR)/a500rom.bin
BOOTBLOCK = $(TOPDIR)/addchip.bootblock.bin
VBRMOVE = $(TOPDIR)/vbrmove

EXTRA-FILES += $(EFFECT).adf
CLEAN-FILES += $(LOADABLES)
CLEAN-FILES += $(EFFECT).exe $(EFFECT).exe.dbg $(EFFECT).exe.map

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

$(TOPDIR)/effects/%.o: FORCE
	$(MAKE) -C $(dir $@) $(notdir $@)

$(TOPDIR)/%.bin: FORCE
	$(MAKE) -C $(dir $@) $(notdir $@)

include $(TOPDIR)/build/common.mk

$(EFFECT).exe.dbg $(EFFECT).exe: $(CRT0) $(MAIN) $(OBJECTS) $(LDEXTRA) $(LDSCRIPT)
	@echo "[LD] $(addprefix $(DIR),$(OBJECTS)) -> $(DIR)$@"
	$(LD) $(LDFLAGS) -L$(TOPDIR)/system -T$(LDSCRIPT) -Map=$@.map -o $@ \
		--start-group $(filter-out %.lds,$^) --end-group
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

data/%.c: data/%.svg
	@echo "[SVG] $(DIR)$< -> $(DIR)$@"
	$(SVG2C) $(SVG2C.$*) -o $@ $<

data/%.c: data/%.2d
	@echo "[2D] $(DIR)$< -> $(DIR)$@"
	$(CONV2D) $(CONV2D.$*) $< > $@ || (rm -f $@ && exit 1)

data/%.c: data/%.sync
	@echo "[SYNC] $(DIR)$< -> $(DIR)$@"
	$(SYNC2C) $(SYNC2C.$*) $< > $@ || (rm -f $@ && exit 1)

ifeq ($(AMIGAOS), 0)
EXTRA-FILES += $(EFFECT).img $(EFFECT).rom
CLEAN-FILES += $(EFFECT).img $(EFFECT).rom

%.img: $(LOADABLES) $(DATA)
	@echo "[IMG] $(addprefix $(DIR),$<) -> $(DIR)$@"
	$(FSUTIL) create $@ $(filter-out %bootloader.bin,$^)

%.adf: %.img $(BOOTLOADER) 
	@echo "[ADF] $(DIR)$< -> $(DIR)$@"
	$(ADFUTIL) -b $(BOOTLOADER) $< $@ 

%.rom: %.img $(ROMSTARTUP)
	@echo "[ROM] $(DIR)$< -> $(DIR)$@"
	$(ROMUTIL) $(ROMSTARTUP) $< $@ 
else
%.adf: %.exe $(BOOTBLOCK)
	@echo "[ADF] $(DIR)$< -> $(DIR)$@"
	echo $< > startup-sequence
	xdftool $@ format dos + write $< + makedir s + write startup-sequence s
	dd if=$(BOOTBLOCK) of=$@ conv=notrunc status=none
	rm startup-sequence
endif

# Default debugger - can be changed by passing DEBUGGER=xyz to make.
DEBUGGER ?= gdb

run-floppy: $(EFFECT).exe.dbg $(EFFECT).adf
	$(LAUNCH) -e $(EFFECT).exe.dbg -f $(EFFECT).adf

debug-floppy: $(EFFECT).exe.dbg $(EFFECT).adf $(TOPDIR)/gdb-dashboard
	$(LAUNCH) -d $(DEBUGGER) -f $(EFFECT).adf -e $(EFFECT).exe.dbg

run: $(EFFECT).rom $(EFFECT).exe.dbg $(EFFECT).adf
	$(LAUNCH) -r $(EFFECT).rom -e $(EFFECT).exe.dbg -f $(EFFECT).adf

debug: $(EFFECT).rom $(EFFECT).exe.dbg $(EFFECT).adf $(TOPDIR)/gdb-dashboard
	$(LAUNCH) -d $(DEBUGGER) -r $(EFFECT).rom -e $(EFFECT).exe.dbg -f $(EFFECT).adf

$(TOPDIR)/gdb-dashboard:
	make -C $(TOPDIR) gdb-dashboard

.PHONY: run debug run-floppy debug-floppy
.PRECIOUS: $(BOOTLOADER) $(BOOTBLOCK) $(EFFECT).img
