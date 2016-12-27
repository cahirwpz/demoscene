libahx.a: ahx.o

AHX-Replayer000.BIN.patched: AHX-Replayer000.BIN AHX-Replayer000.diff
	@echo "FIX  $<"
	../../scripts/binpatch.py $^ $@

ahx.o: ahx.s ../include/ahx.h AHX-Replayer000.BIN.patched

clean::
	@$(RM) -v -r AHX-Replayer000.BIN.patched

include ../Makefile.common
