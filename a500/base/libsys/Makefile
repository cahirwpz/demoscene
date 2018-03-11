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
	keyboard.c \
	memory.c \
	mouse.c \
	qsort.c \
	random.c \
	reader.c \
	serial.c \
	sintab.c \
	sort.c \
	tasks.c

SOURCES_ASM := \
	inflate.s \
	rawio-par.s \
	rawio-ser.s \
	tasks-ll.s

all: libsys.a

include ../../build.mk

libsys.a: $(OBJECTS)

sintab.c: sintab.py
	$(PYTHON) sintab.py
