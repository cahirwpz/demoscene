TOPDIR := $(realpath ../..)

SOURCES := \
	console.c \
	crc32.c \
	file.c \
	fx.c \
	inflate.S \
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
