TOPDIR = $(CURDIR)

export TOPDIR

SUBDIRS = audio engine json gfx p61 std system tinf tools txtgen uvmap 

all:: libs
	@$(MAKE) -C tests
	@$(MAKE) -C effects
	@$(MAKE) -C demo

libs:
	@for subdir in $(SUBDIRS); do $(MAKE) -C $$subdir || exit 1; done

clean-libs:
	@for subdir in $(SUBDIRS); do $(MAKE) -C $$subdir clean; done

clean:: clean-libs
	@$(MAKE) -C tests clean
	@$(MAKE) -C effects clean
	@$(MAKE) -C demo clean

include $(TOPDIR)/Makefile.common

.PHONY: libs clean-libs

# vim: sw=8 ts=8
