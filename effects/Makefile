TOPDIR := $(realpath ..)

SUBDIRS := \
	anim \
	ball \
	bobs3d \
	blurred \
	blurred3d \
	bumpmap-rgb \
	butterfly-gears \
	circles \
	credits \
	empty \
	flatshade \
	flatshade-convex \
	floor \
	floor-old \
	gui \
	glitch \
	highway \
	kbtest \
	layers \
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
	rotator \
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

BUILD-FILES = crt0.o

all: build

include $(TOPDIR)/build/common.mk

run:
	for dir in $(SUBDIRS); do 	\
	  oldcwd=`pwd`;			\
	  cd $$dir && make run;		\
	  cd $$oldcwd;			\
	done

archive:
	7z a "a500-$$(date +%F-%H%M).7z" $(SUBDIRS)

.PHONY: all run archive
