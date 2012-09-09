TOPDIR	= $(CURDIR)

include $(TOPDIR)/Makefile.common

export TOPDIR

BINS	= flares particles raytunnel tunnel vector2d object3d playaudio colormap deformations

all: effects

libs:
	for subdir in $(SUBDIRS); do $(MAKE) -C $$subdir; done

effects: libs
	$(MAKE) -C effects

archive:
	$(MAKE) -C effects archive

clean:
	for subdir in $(SUBDIRS); do $(MAKE) -C $$subdir clean; done
	$(RM) *~ *.o $(BINS)

.PHONY: all archive libs $(BINS)

# vim: sw=8 ts=8
