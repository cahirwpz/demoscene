LIBNAME ?= $(notdir $(CURDIR))

all: build

BUILD-FILES += $(LIBNAME).a

include $(TOPDIR)/build/common.mk

%.a:	$(OBJECTS)
	@echo "[AR] $(addprefix $(DIR),$^) -> $(DIR)$@"
	@m68k-amigaos-ar cr $@ $^
	@m68k-amigaos-ranlib $@
