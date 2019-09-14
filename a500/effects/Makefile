TOPDIR := $(realpath ..)

SUBDIRS := \
	anim \
	ball \
	bobs3d \
	blurred \
	blurred3d \
	bumpmap-rgb \
	circles \
	credits \
	flatshade \
	flatshade-convex \
	floor \
	floor-old \
	gui \
	glitch \
	highway \
	kbtest \
	lines \
	metaballs \
	multipipe \
	neons \
	plasma \
	playahx \
	playctr \
	playp61 \
	plotter \
	prisms \
	shapes \
	showpchg \
	stripes \
	textscroll \
	thunders \
	tiles8 \
	tiles16 \
	tilezoomer \
	transparency \
	twister-rgb \
	uvlight \
	uvmap \
	uvmap-rgb \
	wireframe

FAILURES := \
	tests \
	vscaler
		
all: startup.o $(foreach dir,$(SUBDIRS),build-$(dir))

include $(TOPDIR)/build.mk

clean:: $(foreach dir,$(SUBDIRS),clean-$(dir))

run:
	for dir in $(SUBDIRS); do 	\
	  oldcwd=`pwd`;			\
	  cd $$dir && make run;		\
	  cd $$oldcwd;			\
	done

archive:
	7z a "a500-$$(date +%F-%H%M).7z" $(SUBDIRS)

.PHONY: archive
