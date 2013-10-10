# Warning: P61 player is extremely fragile to changing assembler options. These
# were the only ones that I was able to run the player without crashes.
ASFLAGS	:= -m68000 -nosym

libp61.a: p61.o

p61.o: p61.s p61.h Player61.i

include ../Makefile.common
