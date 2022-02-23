TOPDIR := $(realpath ../..)

CLEAN-FILES := data/start.c

PNG2C.start := --bitmap img_source,320x256x1

include $(TOPDIR)/build/effect.mk

generated.c: script.txt blitzconv.py 
	@echo "[BCONV] $@"
	$(PYTHON3) blitzconv.py  $< > $@