TOPDIR	= $(CURDIR)

include $(TOPDIR)/Makefile.common

export TOPDIR

all: 
	for subdir in $(SUBDIRS); do $(MAKE) -C $$subdir || exit 1; done
	$(MAKE) -C effects
	$(MAKE) -C demo

archive:
	$(MAKE) -C effects archive

clean:
	for subdir in $(SUBDIRS); do $(MAKE) -C $$subdir clean; done
	$(MAKE) -C effects clean
	$(MAKE) -C demo clean
	$(RM) *~ *.o

.PHONY: all archive libs $(BINS)

# vim: sw=8 ts=8
