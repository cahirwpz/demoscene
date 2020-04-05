TOPDIR := $(realpath ../..)

DATA := data/ahxWaves data/ahxWaves.Location
OBJECTS := data/jazzcat-electric_city.o

LIBS := libahx

include $(TOPDIR)/build.effect.mk

data/%.o: data/%.ahx
	@echo "[OBJCOPY] $(DIR)$< -> $(DIR)$@"
	$(OBJCOPY) -I binary -O amiga $^ $@
