TOPDIR := $(realpath ../..)

DATA := data/tiles.png data/tiles-c.png
DATA_GEN := data/twist.png data/twist-colors.png

include $(TOPDIR)/build.effect.mk

data/twist.png: data/twist.py
	@echo "[GEN] $(DIR)$@"
	$(PYTHON3) $^ $@

data/twist-colors.png: data/twist-colors.py
	@echo "[GEN] $(DIR)$@"
	$(PYTHON3) $^ $@
