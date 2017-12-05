SOURCES_C := \
	8svx.c \
	amigahunk.c \
	circle.c \
	color.c \
	common.c \
	console.c \
	coplist.c \
	event.c \
	ffp.c \
	floppy.c \
	fx.c \
	gfx.c \
	hardware.c \
	iff.c \
	ilbm.c \
	interrupts.c \
	io-dos.c \
	io-extra.c \
	io-floppy.c \
	keyboard.c \
	line.c \
	memory.c \
	mouse.c \
	pixmap.c \
	png.c \
	qsort.c \
	random.c \
	reader.c \
	serial.c \
	sintab.c \
	sort.c \
	sound.c \
	sprite.c \
	tasks.c

SOURCES_ASM := \
	c2p_1x1_4.s \
	inflate.s \
	rawio-par.s \
	rawio-ser.s \
	tasks-ll.s

all: libsys.a

include ../../build.mk

libsys.a: $(OBJECTS)

sintab.c: sintab.py
	$(PYTHON) sintab.py
