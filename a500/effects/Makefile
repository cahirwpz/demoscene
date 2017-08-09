TOPDIR := $(realpath ..)

SUBDIRS := \
	anim \
	ball \
	blurred \
	blurred3d \
	bumpmap-rgb \
	circles \
	floor \
	floor-old \
	gui \
	glitch \
	highway \
	kbtest \
	lines \
	metaballs \
	playp61 \
	prisms \
	shapes \
	stripes \
	thunders \
	tilezoomer \
	transparency \
	twister-rgb \
	wireframe

FAILURES := \
	credits \
	flatshade \
	neons \
	playahx \
	plotter \
	tests \
	textscroll \
	uvmap \
	uvmap-rgb \
	vscaler
		
all: startup.o $(foreach dir,$(SUBDIRS),build-$(dir))

include $(TOPDIR)/build.mk

clean:: $(foreach dir,$(SUBDIRS),clean-$(dir))

archive:
	7z a "a500-$$(date +%F-%H%M).7z" $(SUBDIRS)

.PHONY: archive
