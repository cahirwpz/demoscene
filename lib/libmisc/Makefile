TOPDIR := $(realpath ../..)

SOURCES := \
	checksum.asm \
	console.c \
	crc32.c \
	file.c \
	fx.c \
	inflate.S \
	lzsa.asm \
	sintab.c \
	sort.c \
	sync.c \
	uae.S \
	unzx0.asm

SOURCES_GEN := sintab.c

CPPFLAGS.inflate := -DM68020=0

include $(TOPDIR)/build/lib.mk

sintab.c: sintab.py
	$(PYTHON3) sintab.py
