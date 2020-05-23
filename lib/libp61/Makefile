TOPDIR := $(realpath ../..)

# Warning: P61 player is extremely fragile to changing assembler options. These
# were the only ones that I was able to run the player without crashes.
ASFLAGS	:= -m68000 -nosym

SOURCES := p61.s

include $(TOPDIR)/build.lib.mk

p61.o: p61.s $(TOPDIR)/include/p61.h P6112-Play.i
