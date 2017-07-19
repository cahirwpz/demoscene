TOPDIR := $(realpath ..)

SUBDIRS := \
	anim \
	ball \
	blurred \
	bumpmap-rgb \
	circles \
	floor \
	glitch \
	highway \
	kbtest \
	lines \
	metaballs \
	prisms \
	stripes \
	tilezoomer \
	transparency \
	twister-rgb \

FAILURES := \
	blurred3d \
	credits \
	flatshade \
	floor-old \
	gui \
	neons \
	playahx \
	playp61 \
	plotter \
	shapes \
	tests \
	textscroll \
	thunders \
	uvmap \
	uvmap-rgb \
	vscaler \
	wireframe
		
all: startup.o $(foreach dir,$(SUBDIRS),build-$(dir))

include $(TOPDIR)/build.mk

clean:: $(foreach dir,$(SUBDIRS),clean-$(dir))

archive:
	7z a "a500-$$(date +%F-%H%M).7z" $(SUBDIRS)

.PHONY: archive
