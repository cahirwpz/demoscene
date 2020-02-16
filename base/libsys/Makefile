SOURCES_C := \
	amigahunk.c \
	common.c \
	console.c \
	debug-intr.c \
	event.c \
	ffp.c \
	floppy.c \
	fx.c \
	hardware.c \
	iff.c \
	interrupts.c \
	io-dos.c \
	io-extra.c \
	io-floppy.c \
	lib-graphics.c \
	lib-mathffp.c \
	lib-list.c \
	keyboard.c \
	memory.c \
	mouse.c \
	qsort.c \
	print.c \
	random.c \
	reader.c \
	serial.c \
	sintab.c \
	sort.c \
	tasks.c \
	uae.c

SOURCES_ASM := \
	inflate.s \
	rawio-par.s \
	rawio-ser.s \
	tasks-ll.s

all: libsys.a

include ../../build.mk

libsys.a: $(OBJECTS)

sintab.c: sintab.py
	$(PYTHON3) sintab.py

clean::
	$(RM) -f sintab.c
