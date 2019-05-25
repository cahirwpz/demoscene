DATA := twist.png twist-colors.png

all: $(DATA)

twist.png twist-colors.png: twist.py
	@echo "[$(DIR):conv] $< -> $@"
	$(PYTHON3) $^

include ../../build.mk

clean::
	$(RM) $(DATA)
