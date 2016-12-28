DATA = data/running.bin         \
       data/running-pal.ilbm    \

all: anim.exe data/running.bin

data/%.asm: data/%.gif
	./gen-anim.py $< > $@

data/%.bin: data/%.asm
	$(AS) -Fbin -o $@ $<

include ../Makefile.common

clean::
	$(RM) -v -f data/running.bin
