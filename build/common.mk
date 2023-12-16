export TOPDIR

MAKEFLAGS += --no-builtin-rules

DIR := $(patsubst $(TOPDIR)%,%,$(realpath $(CURDIR)))
DIR := $(patsubst /%,%/,$(DIR))

# Compiler tools & flags definitions
CC	:= m68k-amigaos-gcc
CPP	:= m68k-amigaos-cpp
LD	:= m68k-amigaos-ld
AR	:= m68k-amigaos-ar
RANLIB	:= m68k-amigaos-ranlib
VASM	:= vasm -quiet

ASFLAGS	:= -m68010 -Wa,--register-prefix-optional -Wa,--bitwise-or -Wa,-ggdb3
VASMFLAGS	+= -m68010 -quiet -I$(TOPDIR)/include
LDFLAGS	:= -amiga-debug-hunk
CFLAGS	= -ggdb3 -ffreestanding -fno-common $(OFLAGS) $(WFLAGS)
OFLAGS	:= -m68000 -msmall-code -mregparm=2
# The '-O2' option does not turn on optimizations '-funroll-loops',
# '-funroll-all-loops' and `-fstrict-aliasing'.
OFLAGS	+= -O2 -fomit-frame-pointer -fstrength-reduce
WFLAGS	:= -Wall -W -Werror -Wundef -Wsign-compare -Wredundant-decls
WFLAGS	+= -Wnested-externs -Wwrite-strings -Wstrict-prototypes
CPPFLAGS += -I$(TOPDIR)/include
LDSCRIPT := $(TOPDIR)/system/amiga.lds

# Don't reload library base for each call
CPPFLAGS += -D__CONSTLIBBASEDECL__=const
CPPFLAGS += -DCHIPMEM_KB=$(CHIPMEM) -DFASTMEM_KB=$(FASTMEM) -DLOGOUT=$(LOGOUT)
CPPFLAGS += -DPROFILER=$(PROFILER)

include $(TOPDIR)/config.mk

ifeq ($(UAE), 1)
CPPFLAGS += -DUAE
endif

ifeq ($(AMIGAOS), 1)
CPPFLAGS += -DAMIGAOS
endif

ifeq ($(VERBOSE), 0)
.SILENT:
QUIET := --quiet
endif

# Common tools definition
CP := cp -a
RM := rm -v -f
PYTHON3 := PYTHONPATH="$(TOPDIR)/tools/pylib:$$PYTHONPATH" python3
ADFUTIL := $(TOPDIR)/tools/adfutil.py
ROMUTIL := $(TOPDIR)/tools/romutil.py
FSUTIL := $(TOPDIR)/tools/fsutil.py
BINPATCH := $(TOPDIR)/tools/binpatch.py
LAUNCH := $(PYTHON3) $(TOPDIR)/tools/launch.py
LWO2C := $(TOPDIR)/tools/lwo2c.py $(QUIET)
CONV2D := $(TOPDIR)/tools/conv2d.py
GRADIENT := $(TOPDIR)/tools/gradient.py
TMXCONV := $(TOPDIR)/tools/tmxconv/tmxconv
PCHG2C := $(TOPDIR)/tools/pchg2c/pchg2c
PNG2C := $(TOPDIR)/tools/png2c.py
PSF2C := $(TOPDIR)/tools/psf2c.py
SYNC2C := $(TOPDIR)/tools/sync2c/sync2c
STRIP := m68k-amigaos-strip -s
OBJCOPY := m68k-amigaos-objcopy

# Generate dependencies automatically
SOURCES_C = $(filter %.c,$(SOURCES))
SOURCES_S = $(filter %.S,$(SOURCES))
SOURCES_ASM = $(filter %.asm,$(SOURCES))
OBJECTS += $(SOURCES_C:%.c=%.o) $(SOURCES_S:%.S=%.o) $(SOURCES_ASM:%.asm=%.o)

DEPENDENCY-FILES += $(foreach f, $(SOURCES_C),\
		      $(dir $(f))$(patsubst %.c,.%.D,$(notdir $(f))))
DEPENDENCY-FILES += $(foreach f, $(SOURCES_S),\
		      $(dir $(f))$(patsubst %.S,.%.D,$(notdir $(f))))

$(DEPENDENCY-FILES): $(SOURCES_GEN)

CLEAN-FILES += $(DEPENDENCY-FILES) $(SOURCES_GEN) $(OBJECTS) $(DATA_GEN)
CLEAN-FILES += $(SOURCES:%=%~)

# Disable all built-in recipes and define our own
.SUFFIXES:

.%.D: %.c
	@echo "[DEP] $(DIR)$@"
	$(CPP) $(CPPFLAGS) $(CPPFLAGS.$*) -M -MG $< | \
                sed -e 's,$(notdir $*).o,$*.o,g' > $@

.%.D: %.S
	@echo "[DEP] $(DIR)$@"
	$(CPP) $(CPPFLAGS) $(CPPFLAGS.$*) -M -MG $< | \
                sed -e 's,$(notdir $*).o,$*.o,g' > $@

%.o: %.c
	@echo "[CC] $(DIR)$< -> $(DIR)$@"
	$(CC) $(CFLAGS) $(CFLAGS.$*) $(CPPFLAGS) $(CPPFLAGS.$*) -c -o $@ $(CURDIR)/$<

%.o: %.S
	@echo "[AS] $(DIR)$< -> $(DIR)$@"
	$(CC) $(ASFLAGS) $(ASFLAGS.$*) $(CPPFLAGS) $(CPPFLAGS.$*) -c -o $@ $(CURDIR)/$<

%.o: %.asm
	@echo "[VASM] $(DIR)$< -> $(DIR)$@"
	$(VASM) -Fhunk $(VASMFLAGS) $(VASMFLAGS.$*) -o $@ $<

%.bin: %.asm
	@echo "[VASM] $(DIR)$< -> $(DIR)$@"
	$(VASM) -Fbin $(VASMFLAGS) -o $@ $<

%.S: %.c
	@echo "[CC] $(DIR)$< -> $(DIR)$@"
	$(CC) $(CFLAGS) $(CFLAGS.$*) $(CPPFLAGS) $(CPPFLAGS.$*) -fverbose-asm -S -o $@ $<

ifeq ($(words $(findstring $(MAKECMDGOALS), clean)), 0)
  -include $(DEPENDENCY-FILES)
endif

# Rules for recursive build
build-%: FORCE
	@echo "[MAKE] build $(DIR)$*"
	$(MAKE) -C $*

clean-%: FORCE
	@echo "[MAKE] clean $(DIR)$*"
	$(MAKE) -C $* clean

# Rules for build
subdirs: $(foreach dir,$(SUBDIRS),build-$(dir))

build: $(OBJECTS) $(BUILD-FILES) subdirs $(EXTRA-FILES) 

clean: $(foreach dir,$(SUBDIRS),clean-$(dir)) 
	$(RM) $(BUILD-FILES) $(EXTRA-FILES) $(CLEAN-FILES) *~ *.taghl

.PHONY: all build subdirs clean FORCE

# vim: ts=8 et
