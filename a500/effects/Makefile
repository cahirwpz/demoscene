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
	glitch \
	highway \
	kbtest \
	lines \
	metaballs \
	neons \
	playahx \
	playp61 \
	plotter \
	prisms \
	shapes \
	stripes \
	tests \
	textscroll \
	thunders \
	tilezoomer \
	transparency \
	twister-rgb \
	uvmap \
	uvmap-rgb \
	vscaler \
	wireframe

	# gui 
		
all: startup.o $(foreach dir,$(SUBDIRS),build-$(dir))

include $(TOPDIR)/build.mk

clean:: $(foreach dir,$(SUBDIRS),clean-$(dir))

archive:
	7z a "a500-$$(date +%F-%H%M).7z" $(SUBDIRS)

.PHONY: archive
