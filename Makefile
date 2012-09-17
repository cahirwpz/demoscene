TOPDIR = $(CURDIR)

export TOPDIR

all::
	@for subdir in $(SUBDIRS); do $(MAKE) -C $$subdir || exit 1; done
	@$(MAKE) -C effects
	@$(MAKE) -C demo

clean::
	@for subdir in $(SUBDIRS); do $(MAKE) -C $$subdir clean; done
	@$(MAKE) -C effects clean
	@$(MAKE) -C demo clean

include $(TOPDIR)/Makefile.common

# vim: sw=8 ts=8
