TOPDIR := $(realpath ../..)

SOURCES := \
	amigahunk.c \
	audio.c \
	common.c \
	console.c \
	debug-intr.c \
	event.c \
	ffp.c \
	floppy.c \
	fx.c \
	hardware.c \
	iff.c \
	inflate.s \
	interrupts.c \
	io-dos.c \
	io-extra.c \
	io-floppy.c \
	keyboard.c \
	lib-graphics.c \
	lib-list.c \
	lib-mathffp.c \
	memory.c \
	mouse.c \
	print.c \
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
