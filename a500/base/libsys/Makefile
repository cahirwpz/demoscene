SOURCES_C := \
	8svx.c \
	amigahunk.c \
	blitter.c \
	blt-add-sat.c \
	blt-copy-area.c \
	blt-copy-fast.c \
	blt-copy-mask.c \
	blt-copy.c \
	blt-dec-sat.c \
	blt-fill-area.c \
	blt-inc-sat.c \
	blt-line.c \
	blt-make-mask.c \
	blt-set-area.c \
	blt-set-mask-area.c \
	circle.c \
	color.c \
	common.c \
	console.c \
	coplist.c \
	event.c \
	ffp.c \
	floppy.c \
	font-load.c \
	font.c \
	fx.c \
	gfx.c \
	gui.c \
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
	sort.c \
	sound.c \
	sprite.c \
	tasks.c

SOURCES_ASM := \
	c2p_1x1_4.s \
	inflate.s \
	rawio-par.s \
	rawio-ser.s \
	sintab.s

all: libsys.a

include ../../build.mk

libsys.a: $(OBJECTS)

sintab.s: sintab.py
	python sintab.py
