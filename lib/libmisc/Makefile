TOPDIR := $(realpath ../..)

SOURCES := \
	console.c \
	debug.c \
	debug-putchar.S \
	fx.c \
	inflate.S \
	sintab.c \
	sort.c \
	uae.asm

SOURCES_GEN := sintab.c

CPPFLAGS.inflate := -DM68020=0

include $(TOPDIR)/build/lib.mk

sintab.c: sintab.py
	$(PYTHON3) sintab.py
