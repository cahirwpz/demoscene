LIBNAME = $(notdir $(PWD))

BUILD-FILES += $(LIBNAME).a
CLEAN-FILES +=

all: build

include $(TOPDIR)/build/common.mk

%.a:	$(OBJECTS)
	@echo "[AR] $(addprefix $(DIR),$^) -> $(DIR)$@"
	@m68k-amigaos-ar cr $@ $^
	@m68k-amigaos-ranlib $@
