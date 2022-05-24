TOPDIR := $(realpath ../..)

# Warning: P61 player is extremely fragile to changing assembler options. These
# were the only ones that I was able to run the player without crashes.
VASMFLAGS := -m68000 -nosym

SOURCES := p61.asm

include $(TOPDIR)/build/lib.mk

p61.o: p61.asm $(TOPDIR)/include/p61.h P6112-Play.i
