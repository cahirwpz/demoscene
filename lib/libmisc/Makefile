TOPDIR := $(realpath ../..)

SOURCES := \
	audio.c \
	console.c \
	debug.c \
	fx.c \
	inflate.asm \
	lzo.c \
	qsort.c \
	rawio-par.asm \
	sintab.c \
	sort.c \
	uae.asm

SOURCES_GEN := sintab.c

include $(TOPDIR)/build/lib.mk

sintab.c: sintab.py
	$(PYTHON3) sintab.py
