TOPDIR := $(realpath ../..)

SOURCES := \
	amigahunk.c \
	audio.c \
	common.c \
	console.c \
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
	memory.c \
	mouse.c \
	qsort.c \
	random.c \
	rawio-par.s \
	rawio-ser.s \
	reader.c \
	serial.c \
	sintab.c \
	sort.c \
	tasks-ll.s \
	tasks.c \
	uae.c

SOURCES_GEN := sintab.c

include $(TOPDIR)/build.lib.mk

sintab.c: sintab.py
	$(PYTHON3) sintab.py
