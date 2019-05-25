IMAGES := $(subst .png,.ilbm,$(wildcard greet_*.png)) neons.ilbm neons-2.ilbm

all: $(IMAGES)

include ../../build.mk

%.png: %.py
	@echo "[$(DIR):gen] $< -> $@"
	$(PYTHON3) $<

clean::
	$(RM) $(IMAGES)
