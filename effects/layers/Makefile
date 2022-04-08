TOPDIR := $(realpath ../..)

CLEAN-FILES := \
	data/background.c data/foreground.c \
	data/bg-gradient.c data/fg-gradient.c \
	data/bg-gradient.png data/fg-gradient.png

PNG2C.background := --bitmap background,384x384x3
PNG2C.foreground := --bitmap foreground,384x384x3
PNG2C.bg-gradient := --pixmap bg_gradient,7x61x12
PNG2C.fg-gradient := --pixmap fg_gradient,6x61x12

include $(TOPDIR)/build/effect.mk

GRDCMD = copy hue 0.25 copy hue 0.25 copy hue 0.25 copy hue 0.25

data/bg-gradient.png: $(GRADIENT) data/background.png
	@echo "[GEN] $@"
	$(PYTHON3) $(GRADIENT) load $(word 2,$^) val 0.25 \
		$(GRDCMD) save $@

data/fg-gradient.png: $(GRADIENT) data/foreground.png
	@echo "[GEN] $@"
	$(PYTHON3) $(GRADIENT) load $(word 2,$^) sat 0.25 hue 0.5 \
		$(GRDCMD) save $@
