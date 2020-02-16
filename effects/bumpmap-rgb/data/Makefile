all: light.png bumpmap.bin

include ../../build.mk

bumpmap.bin: bumpmap.py bumpmap.png
	@echo "[$(DIR):gen] $< -> $@"
	$(PYTHON3) bumpmap.py bumpmap.png

light.png: light.py
	@echo "[$(DIR):gen] $< -> $@"
	$(PYTHON3) light.py

clean::
	$(RM) light.png bumpmap.bin
