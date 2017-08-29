all: running.bin

include ../../build.mk

%.asm: %.gif
	@echo "[$(DIR):gen] $< -> $@"
	$(PYTHON) gen-anim.py $< > $@

clean::
	$(RM) running.bin running.asm

.PRECIOUS: %.asm
