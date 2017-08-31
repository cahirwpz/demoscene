all: plotter-flares.ilbm

include ../../build.mk

plotter-flares.png:
	@echo "[$(DIR):gen] $@"
	$(PYTHON) plotter-flares.py
	$(OPTIPNG) $@

clean::
	$(RM) plotter-flares.{ilbm,png}
