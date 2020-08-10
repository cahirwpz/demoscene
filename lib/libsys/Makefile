TOPDIR := $(realpath ../..)

SOURCES := \
	amigahunk.c \
	audio.c \
	console.c \
	debug.c \
	debug-intr.c \
	event.c \
	floppy.c \
	fx.c \
	hardware.c \
	inflate.s \
	interrupts.c \
	io.c \
	io-extra.c \
	keyboard.c \
	lzo.c \
	memdump.c \
	memory.c \
	mouse.c \
	qsort.c \
	rawio-par.s \
	rawio-ser.s \
	serial.c \
	sintab.c \
	sort.c \
	tasks-ll.s \
	tasks.c \
	uae.s

SOURCES_GEN := sintab.c

include $(TOPDIR)/build/lib.mk

sintab.c: sintab.py
	$(PYTHON3) sintab.py
