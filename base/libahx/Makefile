SOURCES_S := ahx.s

all: libahx.a

include ../../build.mk

libahx.a: ahx.o

AHX-Replayer000.BIN.patched: AHX-Replayer000.BIN AHX-Replayer000.diff
	@echo "[$(DIR):fix] $^ -> $@"
	$(BINPATCH) $^ $@

ahx.o: ahx.s ../include/ahx.h AHX-Replayer000.BIN.patched

clean::
	@$(RM) AHX-Replayer000.BIN.patched
