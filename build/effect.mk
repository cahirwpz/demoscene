EFFECT := $(notdir $(CURDIR))

ifndef SOURCES
SOURCES = $(EFFECT).c
endif

LIBS += libblit libgfx libsys libc
CPPFLAGS += -I$(TOPDIR)/effects
LDEXTRA = $(TOPDIR)/effects/libeffect.a
LDEXTRA += $(foreach lib,$(LIBS),$(TOPDIR)/lib/$(lib)/$(lib).a)

CRT0 = $(TOPDIR)/effects/crt0.o
BOOTLOADER = $(TOPDIR)/bootloader.bin

EXTRA-FILES += $(DATA_GEN) $(EFFECT).exe $(EFFECT).adf
CLEAN-FILES += $(DATA_GEN) $(EFFECT).exe.dbg $(EFFECT).exe.map 

all: build

# Check if library is up-to date if someone is asking explicitely
$(TOPDIR)/lib/lib%.a: FORCE
	$(MAKE) -C $(dir $@) $(notdir $@)

$(TOPDIR)/effects/%.o: FORCE
	$(MAKE) -C $(dir $@) $(notdir $@)

$(TOPDIR)/effects/%.a: FORCE
	$(MAKE) -C $(dir $@) $(notdir $@)

$(TOPDIR)/%.bin: FORCE
	$(MAKE) -C $(dir $@) $(notdir $@)

include $(TOPDIR)/build/common.mk

$(EFFECT).exe: $(CRT0) $(OBJECTS) $(LDEXTRA)
	@echo "[LD] $(addprefix $(DIR),$(OBJECTS)) -> $(DIR)$@"
	$(CC) $(LDFLAGS) -Wl,-Map=$@.map -o $@ $^ $(LDLIBS)
	$(CP) $@ $@.dbg
	$(STRIP) $@

data/%.c: data/%.lwo
	@echo "[LWO] $(DIR)$< -> $(DIR)$@"
	$(LWO2C) $(LWO2C.$*) -f $< $@

data/%.c: data/%.psfu
	@echo "[PSF] $(DIR)$^ -> $(DIR)$@"
	$(PSF2C) $(PSF2C.$*) $< > $@

data/%.c: data/%.png
	@echo "[PNG] $(DIR)$< -> $(DIR)$@"
	$(PNG2C) $(PNG2C.$*) $< > $@

data/%.c: data/%.2d
	@echo "[2D] $(DIR)$< -> $(DIR)$@"
	$(CONV2D) $(CONV2D.$*) $< > $@

%.adf: %.exe $(DATA) $(DATA_GEN) $(BOOTLOADER)
	@echo "[ADF] $(addprefix $(DIR),$*.exe $(DATA) $(DATA_GEN)) -> $(DIR)$@"
	$(FSUTIL) -b $(BOOTLOADER) create $@ $^

run: all $(notdir $(PWD)).adf
	$(LAUNCH) -e $(notdir $(PWD)).exe.dbg -f $(lastword $^)

.PHONY: run
.PRECIOUS: $(BOOTLOADER)
