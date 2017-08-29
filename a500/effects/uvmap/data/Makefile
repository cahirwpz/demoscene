all: uvmap.png

include ../../build.mk

uvmap.png: gen-uvmap.py
	@echo "[$(DIR):gen] $< -> $@"
	$(PYTHON) gen-uvmap.py $< > $@

clean::
	$(RM) uvmap.png uvmap-[uv].png
