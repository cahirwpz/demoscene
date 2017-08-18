TOPDIR := $(realpath ..)

SUBDIRS := \
	anim \
	ball \
	blurred \
	blurred3d \
	bumpmap-rgb \
	circles \
	credits \
	flatshade \
	floor \
	floor-old \
	gui \
	glitch \
	highway \
	kbtest \
	lines \
	metaballs \
	neons \
	plasma \
	playp61 \
	plotter \
	prisms \
	shapes \
	stripes \
	textscroll \
	thunders \
	tiles8 \
	tiles16 \
	tilezoomer \
	transparency \
	twister-rgb \
	uvmap \
	uvmap-rgb \
	wireframe

FAILURES := \
	playahx \
	tests \
	vscaler
		
all: startup.o $(foreach dir,$(SUBDIRS),build-$(dir))

include $(TOPDIR)/build.mk

clean:: $(foreach dir,$(SUBDIRS),clean-$(dir))

archive:
	7z a "a500-$$(date +%F-%H%M).7z" $(SUBDIRS)

.PHONY: archive
