all: running.bin running-pal.ilbm

include ../../build.mk

%.asm: %.gif
	@echo "[$(DIR):gen] $< -> $@"
	$(PYTHON) gen-anim.py $< > $@

clean::
	$(RM) running.bin running.asm running-pal.ilbm

.PRECIOUS: %.asm
