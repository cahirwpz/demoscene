all: anim.exe data/running.bin

data/running.asm: data/running.gif
	./gen-anim.py $< > $@

data/running.bin: data/running.asm
	$(AS) -Fbin -o $@ $<

include ../Makefile.common

clean::
	$(RM) -f data/running.{bin,asm}
