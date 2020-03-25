EFFECT := $(notdir $(CURDIR))

ifndef SOURCES
SOURCES = $(EFFECT).c
endif

LIBS += libblit libgfx libsys libc
CPPFLAGS += -I$(TOPDIR)/effects
LDEXTRA = $(foreach lib,$(LIBS),$(TOPDIR)/base/$(lib)/$(lib).a)
STARTUP = $(TOPDIR)/effects/startup.o 

BUILD-FILES += $(DATA_GEN) $(EFFECT).exe $(EFFECT).adf
CLEAN-FILES += $(DATA_GEN) $(EFFECT).exe.dbg $(EFFECT).exe.map 

all: build

include $(TOPDIR)/build.mk

$(EFFECT).exe: $(CRT0) $(OBJECTS) $(STARTUP) $(LDEXTRA)
	@echo "[LD] $(addprefix $(DIR),$(OBJECTS)) -> $(DIR)$@"
	$(CC) $(LDFLAGS) -Wl,-Map=$@.map -o $@ $^ $(LDLIBS)
	$(CP) $@ $@.dbg
	$(STRIP) $@

# Check if library is up-to date if someone is asking explicitely
$(TOPDIR)/base/lib%.a: FORCE
	$(MAKE) -C $(dir $@) $(notdir $@)

$(TOPDIR)/base/%.o: FORCE
	$(MAKE) -C $(dir $@) $(notdir $@)

$(TOPDIR)/base/%.bin: FORCE
	$(MAKE) -C $(dir $@) $(notdir $@)

$(TOPDIR)/effects/%.o: FORCE
	$(MAKE) -C $(dir $@) $(notdir $@)

%.3d: %.lwo
	@echo "[LWO] $(DIR)$< -> $(DIR)$@"
	$(DUMPLWO) -f $< $@

%.ilbm: %.png
	@echo "[ILBM] $(DIR)$< -> $(DIR)$@"
	$(ILBMCONV) $< $@
	$(ILBMPACK) -f $@

%.png: %.psfu
	@echo "[PSF] $(DIR)$< -> $(DIR)$@"
	$(PSFTOPNG) $<
	$(OPTIPNG) $@

data/%.c: data/%.png
	@echo "[PNG] $(DIR)$< -> $(DIR)$@"
	$(PNG2C) $(PNG2C.$*) $< > $@

%.bin: %.asm
	@echo "[BIN] $(DIR)$< -> $(DIR)$@"
	$(AS) -Fbin -o $@ $<

%.h %-map.bin %-tiles.png: %.tmx
	@echo "[TMX] $(DIR)$< -> $(addprefix $(DIR),$*.h $*-map.bin $*-tiles.png)"
	$(TMXCONV) $< > $@

%.adf: %.exe $(DATA) $(DATA_GEN) $(TOPDIR)/base/bootloader.bin
	@echo "[ADF] $(addprefix $(DIR),$*.exe $(DATA) $(DATA_GEN)) -> $(DIR)$@"
	$(FSUTIL) -b $(TOPDIR)/base/bootloader.bin create $@ $^

run: all $(notdir $(PWD)).adf
	$(RUNINUAE) -e $(notdir $(PWD)).exe.dbg $(lastword $^)

.PHONY: run