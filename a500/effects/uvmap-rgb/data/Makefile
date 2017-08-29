all: uvmap-rgb.png

include ../../build.mk

uvmap-rgb.png: gen-uvmap.py
	@echo "[$(DIR):gen] $< -> $@"
	$(PYTHON) gen-uvmap.py $< > $@

clean::
	$(RM) uvmap-rgb.png uvmap-rgb-[uv].png
