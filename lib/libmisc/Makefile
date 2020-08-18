TOPDIR := $(realpath ../..)

SOURCES := \
	console.c \
	debug.c \
	debug-putchar.S \
	fx.c \
	inflate.asm \
	lzo.c \
	qsort.c \
	sintab.c \
	sort.c \
	uae.asm

SOURCES_GEN := sintab.c

include $(TOPDIR)/build/lib.mk

sintab.c: sintab.py
	$(PYTHON3) sintab.py
