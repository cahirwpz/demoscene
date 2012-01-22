# Warning: P61 player is extremely fragile to changing assembler options. These
# were the only ones that I was able to run the player without crashes.

AS = vasmm68k_mot
ASFLAGS	= -Fhunk -m68000 -nosym

p61.o: p61.s p61.h Player61.i
	$(AS) $(ASFLAGS) -o $@ $<

clean:
	$(RM) *.o *~

.PHONY: clean
