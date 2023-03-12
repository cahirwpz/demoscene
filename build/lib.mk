LIBNAME ?= $(notdir $(CURDIR))

all: build

BUILD-FILES += $(LIBNAME).a

include $(TOPDIR)/build/common.mk

%.a:	$(OBJECTS)
	@echo "[AR] $(addprefix $(DIR),$^) -> $(DIR)$@"
	@truncate -s 0 $@
	@$(AR) cr $@ $^
	@$(RANLIB) $@
