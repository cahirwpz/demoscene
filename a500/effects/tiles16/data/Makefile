DATA := tiles.png tiles.dat tiles.ilbm

all: $(DATA)

tiles.png tiles.dat: gen-tiles.py Tiny16.png
	@echo "[$(DIR):conv] $^ -> $@"
	$(PYTHON) $^

include ../../build.mk

clean::
	$(RM) $(DATA)
