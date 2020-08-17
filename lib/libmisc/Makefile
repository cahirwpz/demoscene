TOPDIR := $(realpath ../..)

SOURCES := \
	audio.c \
	console.c \
	debug.c \
	fx.c \
	inflate.asm \
	lzo.c \
	memdump.c \
	qsort.c \
	rawio-par.asm \
	rawio-ser.asm \
	sintab.c \
	sort.c \
	uae.asm

SOURCES_GEN := sintab.c

include $(TOPDIR)/build/lib.mk

sintab.c: sintab.py
	$(PYTHON3) sintab.py
