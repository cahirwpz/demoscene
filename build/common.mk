export TOPDIR

MAKEFLAGS += --no-builtin-rules

DIR := $(patsubst $(TOPDIR)%,%,$(realpath $(CURDIR)))
DIR := $(patsubst /%,%/,$(DIR))

# Compiler tools & flags definitions
CC	:= m68k-amigaos-gcc -g -ffreestanding -noixemul 
AS	:= vasm -quiet
CFLAGS	= $(LDFLAGS) $(OFLAGS) $(WFLAGS) $(DFLAGS)

ASFLAGS	:= -x -m68010
LDFLAGS	:= -g -m68000 -msmall-code -nostartfiles -nostdlib -nodefaultlibs
# The '-O2' option does not turn on optimizations '-funroll-loops',
# '-funroll-all-loops' and `-fstrict-aliasing'.
OFLAGS	:= -O2 -fomit-frame-pointer -fstrength-reduce -mregparm=2
WFLAGS	:= -Wall -W -Werror -Wundef -Wsign-compare -Wredundant-decls
WFLAGS	+= -Wnested-externs -Wwrite-strings -Wstrict-prototypes
DFLAGS	+= -DUAE

# Default configuration
DEFAULT_CHIP_CHUNK ?= 262144
DEFAULT_FAST_CHUNK ?= 262144
FRAMES_PER_ROW ?= 6

DFLAGS	+= -DDEFAULT_CHIP_CHUNK=$(DEFAULT_CHIP_CHUNK)
DFLAGS	+= -DDEFAULT_FAST_CHUNK=$(DEFAULT_FAST_CHUNK)
DFLAGS	+= -DFRAMES_PER_ROW=$(FRAMES_PER_ROW)

# Pass "VERBOSE=1" at command line to display command being invoked by GNU Make
ifneq ($(VERBOSE), 1)
.SILENT:
QUIET := --quiet
endif

# Don't reload library base for each call.
DFLAGS += -D__CONSTLIBBASEDECL__=const

CPPFLAGS += -I$(TOPDIR)/include

# Common tools definition
CP := cp -a
RM := rm -v -f
PYTHON3 := PYTHONPATH="$(TOPDIR)/tools/pylib:$$PYTHONPATH" python3
FSUTIL := $(TOPDIR)/tools/fsutil.py
BINPATCH := $(TOPDIR)/tools/binpatch.py
LAUNCH := $(PYTHON3) $(TOPDIR)/tools/launch.py
LWO2C := $(TOPDIR)/tools/lwo2c.py $(QUIET)
CONV2D := $(TOPDIR)/tools/conv2d.py
TMXCONV := $(TOPDIR)/tools/tmxconv/tmxconv
PCHG2C := $(TOPDIR)/tools/pchg2c/pchg2c
PNG2C := $(TOPDIR)/tools/png2c.py
PSF2C := $(TOPDIR)/tools/psf2c.py
SYNC2C := $(TOPDIR)/tools/sync2c/sync2c
STRIP := m68k-amigaos-strip -s
OBJCOPY := m68k-amigaos-objcopy

# Generate dependencies automatically
SOURCES_C = $(filter %.c,$(SOURCES))
SOURCES_ASM = $(filter %.s,$(SOURCES))
OBJECTS += $(SOURCES_C:.c=.o) $(SOURCES_ASM:.s=.o)
DEPFILES = $(SOURCES_C:%.c=.%.P)

$(DEPFILES): $(SOURCES_GEN)

ifeq ($(words $(findstring $(MAKECMDGOALS), clean)), 0)
  -include $(DEPFILES)
endif

CLEAN-FILES += $(DEPFILES) $(SOURCES_GEN) $(OBJECTS) $(DATA_GEN)
CLEAN-FILES += $(SOURCES_C:%=%~) $(SOURCES_ASM:%=%~)

# Disable all built-in recipes and define our own
.SUFFIXES:

.%.P: %.c
	@echo "[DEP] $(DIR)$< -> $(DIR)$@"
	$(CC) $(CPPFLAGS) -MM -MG -o $@ $<

%.o: %.c .%.P
	@echo "[CC] $(DIR)$< -> $(DIR)$@"
	$(CC) $(CPPFLAGS) $(CFLAGS) -c -o $@ $<

%.o: %.s
	@echo "[AS] $(DIR)$< -> $(DIR)$@"
	$(AS) -Fhunk $(ASFLAGS) -o $@ $<

%.bin: %.s
	@echo "[AS] $(DIR)$< -> $(DIR)$@"
	$(AS) -Fbin $(ASFLAGS) -o $@ $<

%.s: %.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -S -fverbose-asm -o $@ $<

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
