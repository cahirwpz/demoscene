TOPDIR := $(realpath ..)

# the order here must be the same as in demo.json !!!
SUBDIRS := loader playpt

SOURCES := main.c
LOADABLES := cockbusters.exe $(foreach dir, $(SUBDIRS), $(dir).exe)
MAIN := #

DEMO := 1
export DEMO

CPPFLAGS += -I.
LDFLAGS_EXTRA :=
LDSCRIPT := demo.lds
LIBS := libpt lib3d

include $(TOPDIR)/build/effect.mk

$(LOADABLES): demo.exe $(foreach dir, $(SUBDIRS), $(dir)/$(dir).exe)
	@echo "[SPLITEXE] $< -> $(LOADABLES)"
	$(SPLITEXE) $< $(LOADABLES)
	$(PACKEXE) -algo zx0 $(firstword $(LOADABLES))
	for exe in $(foreach dir, $(SUBDIRS), $(dir).exe); do \
		$(PACKEXE) -algo lzsa1 $$exe; \
	done

demo.lds: demo.json
	@echo "[LDS] $< -> $@"
	$(PYTHON3) gen-lds.py $< $@ || (rm -f $@ && exit 1)

demo.c: demo.sync
	@echo "[SYNC] $< -> $@"
	$(SYNC2C) $< > $@ || (rm -f $@ && exit 1)

CLEAN-FILES += demo.lds demo.c
