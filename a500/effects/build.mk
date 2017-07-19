TOPDIR = $(realpath $(dir $(lastword $(MAKEFILE_LIST)))/..)

include $(TOPDIR)/build.mk

LIBS += libsys libc
CPPFLAGS += -I$(TOPDIR)/effects
LDLIBS += $(foreach lib,$(LIBS),$(TOPDIR)/base/$(lib)/$(lib).a)

%.exe: $(TOPDIR)/base/crt0.o %.o $(TOPDIR)/effects/startup.o
	@echo "[$(DIR):ld] $@"
	$(CC) $(LDFLAGS) -o $@ $^ $(LDLIBS)

%.3d: %.lwo
	@echo "[$(DIR):conv] $< -> $@"
	$(DUMPLWO) $< $@

%.ilbm: %.png
	@echo "[$(DIR):conv] $< -> $@"
	$(ILBMCONV) $< $@
	$(ILBMPACK) -f $@

%.bin: %.asm
	@echo "[$(DIR):bin] $< -> $@"
	$(AS) -Fbin -o $@ $<

%.adf: %.exe $(DATA)
	$(FSUTIL) -b $(TOPDIR)/base/bootloader.bin create $@ $^

run: all $(notdir $(PWD)).adf
	$(RUNINUAE) $(lastword $^)

clean::
	@$(RM) *.adf *.exe

.PHONY: run
.PRECIOUS: %.o
