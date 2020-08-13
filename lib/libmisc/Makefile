TOPDIR := $(realpath ../..)

SOURCES := \
	audio.c \
	console.c \
	debug.c \
	fx.c \
	inflate.s \
	lzo.c \
	memdump.c \
	qsort.c \
	rawio-par.s \
	rawio-ser.s \
	sintab.c \
	sort.c \
	uae.s

SOURCES_GEN := sintab.c

include $(TOPDIR)/build/lib.mk

sintab.c: sintab.py
	$(PYTHON3) sintab.py
