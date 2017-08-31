TOPDIR := $(realpath $(dir $(lastword $(MAKEFILE_LIST))))

# Compiler tools & flags definitions
CC	:= m68k-amigaos-gcc -noixemul -g
AS	:= vasm -quiet
CFLAGS	= $(LDFLAGS) $(OFLAGS) $(WFLAGS) $(DFLAGS)

ASFLAGS	:= -x -m68010
LDFLAGS	:= -m68000 -msmall-code -nostartfiles
# The '-O2' option does not turn on optimizations '-funroll-loops',
# '-funroll-all-loops' and `-fstrict-aliasing'.
OFLAGS	:= -O2 -fomit-frame-pointer -fstrength-reduce
WFLAGS	:= -Wall -Werror

# Pass "VERBOSE=1" at command line to display command being invoked by GNU Make
ifneq ($(VERBOSE), 1)
.SILENT:
QUIET := --quiet
endif

# Don't reload library base for each call.
DFLAGS := -D__CONSTLIBBASEDECL__=const

# libnix13.a contains a few functions that don't depend on utility.library
# which is not present in Kickstart 1.3
LDLIBS	+= -lnix13 -lstubs
CPPFLAGS += -I$(TOPDIR)/base/include

# Common tools definition
CP := cp -a
RM := rm -v -f
PYTHON := PYTHONPATH="$(TOPDIR)/pylib:$$PYTHONPATH" python2 
PYTHON3 := PYTHONPATH="$(TOPDIR)/pylib:$$PYTHONPATH" python3
FSUTIL := $(TOPDIR)/tools/fsutil.py
BINPATCH := $(TOPDIR)/tools/binpatch.py
RUNINUAE := $(PYTHON3) $(TOPDIR)/effects/RunInUAE
ILBMCONV := $(TOPDIR)/tools/ilbmconv.py
ILBMPACK := $(TOPDIR)/tools/ilbmpack.py $(QUIET)
DUMPLWO := $(TOPDIR)/tools/dumplwo.py $(QUIET)
PSFTOPNG := $(TOPDIR)/tools/psftopng.py
OPTIPNG := optipng $(QUIET)
STRIP := m68k-amigaos-strip -s

# Rules for recursive build
DIR := $(notdir $(patsubst $(TOPDIR)/%,%,$(CURDIR)))

build-%:
	$(MAKE) -C $(@:build-%=%)

clean-%:
	$(MAKE) -C $(@:clean-%=%) clean

# Generate dependencies automatically
SOURCES = $(SOURCES_C) $(SOURCES_ASM)
OBJECTS = $(SOURCES_C:.c=.o) $(SOURCES_ASM:.s=.o)
DEPFILES = $(SOURCES_C:%.c=.%.P)

ifeq ($(words $(findstring $(MAKECMDGOALS), clean)), 0)
  -include $(DEPFILES)
endif

# Disable all built-in recipes and define our own
.SUFFIXES:

.%.P: %.c
	@echo "[$(DIR):dep] $< -> $@"
	$(CC) $(CPPFLAGS) -MM -MG -o $@ $<

%.o: %.c .%.P
	@echo "[$(DIR):cc] $< -> $@"
	$(CC) $(CPPFLAGS) $(CFLAGS) -c -o $@ $<

%: %.o
	@echo "[$(DIR):ld] $^ -> $@"
	$(CC) $(LDFLAGS) -o $@ $^ $(LDLIBS)

%.o: %.s
	@echo "[$(DIR):as] $< -> $@"
	$(AS) -Faout $(ASFLAGS) -o $@ $<

%.bin: %.s
	@echo "[$(DIR):as] $< -> $@"
	$(AS) -Fbin $(ASFLAGS) -o $@ $<

%.a:	$(OBJECTS)
	@echo "[$(DIR):ar] $^ -> $@"
	@m68k-amigaos-ar cr $@ $^
	@m68k-amigaos-ranlib $@

%.s: %.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -S -fverbose-asm -o $@ $<

clean::
	@$(RM) .*.P *.a *.o *~ *.exe *.exe.dbg *.exe.map *.taghl

.PHONY: all clean
