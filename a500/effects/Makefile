TOPDIR := $(realpath ..)

SUBDIRS := \
	anim \
	ball \
	blurred \
	bumpmap-rgb \
	circles \
	floor \
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
	tilezoomer \
	transparency \
	twister-rgb \

FAILURES := \
	blurred3d \
	credits \
	flatshade \
	floor-old \
	neons \
	playahx \
	plotter \
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
