DATA := data/running.bin \
	data/running-pal.ilbm

all: anim.exe data/running.bin

include ../build.mk

data/%.asm: data/%.gif
	@echo "[$(DIR):gen] $< -> $@"
	./gen-anim.py $< > $@

data/%.bin: data/%.asm
	@echo "[$(DIR):bin] $< -> $@"
	$(AS) -Fbin -o $@ $<

clean::
	$(RM) anim.exe data/running.bin
