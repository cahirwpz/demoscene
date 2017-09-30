all: torus-map.png

include ../../build.mk

torus-map.png: gen-uvmap.py
	@echo "[$(DIR):gen] $< -> $@"
	$(PYTHON) gen-uvmap.py

clean::
	$(RM) torus-map.png
