all: light.png bumpmap.bin

include ../../build.mk

bumpmap.bin: bumpmap.py bumpmap.png
	@echo "[$(DIR):gen] $< -> $@"
	$(PYTHON) bumpmap.py bumpmap.png

light.png: light.py
	@echo "[$(DIR):gen] $< -> $@"
	$(PYTHON) light.py

clean::
	$(RM) light.png bumpmap.bin
