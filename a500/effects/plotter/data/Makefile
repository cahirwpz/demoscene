all: plotter-flares.ilbm

include ../../build.mk

plotter-flares.png:
	@echo "[$(DIR):gen] $@"
	$(PYTHON3) plotter-flares.py
	$(OPTIPNG) $@

clean::
	$(RM) plotter-flares.{ilbm,png}
