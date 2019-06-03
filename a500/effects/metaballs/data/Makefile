IMAGES := \
	metaball-bg-left-1.ilbm \
	metaball-bg-left-2.ilbm \
	metaball-bg-right-1.ilbm \
	metaball-bg-right-2.ilbm \
	metaball.png metaball.ilbm

all: $(IMAGES)

include ../../build.mk

metaball.png: gen-metaball.py
	@echo "[$(DIR):gen] $< -> $@"
	$(PYTHON3) $<

clean::
	$(RM) $(IMAGES)
