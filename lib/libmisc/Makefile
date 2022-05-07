TOPDIR := $(realpath ../..)

SOURCES := \
	console.c \
	fx.c \
	inflate.S \
	sintab.c \
	sort.c \
	sync.c \
	uae.S

SOURCES_GEN := sintab.c

CPPFLAGS.inflate := -DM68020=0

include $(TOPDIR)/build/lib.mk

sintab.c: sintab.py
	$(PYTHON3) sintab.py
