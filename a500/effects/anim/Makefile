all: anim.exe data/running.bin

data/%.asm: data/%.gif
	./gen-anim.py $< > $@

data/%.bin: data/%.asm
	$(AS) -Fbin -o $@ $<

include ../Makefile.common

clean::
	$(RM) -f data/running.{bin,asm}
