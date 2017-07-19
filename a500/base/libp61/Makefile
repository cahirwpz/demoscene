# Warning: P61 player is extremely fragile to changing assembler options. These
# were the only ones that I was able to run the player without crashes.
ASFLAGS	:= -m68000 -nosym

SOURCES_S := p61.s

all: libp61.a

include ../../build.mk

libp61.a: p61.o

p61.o: p61.s ../include/p61.h P6112-Play.i
