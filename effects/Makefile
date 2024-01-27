TOPDIR := $(realpath ..)

SUBDIRS := \
	anim \
	anim-polygons \
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
	fire-rgb \
	game-of-life \
	glitch \
	growing-tree \
	gui \
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
	playpt \
	plotter \
	prisms \
	rotator \
	sea-anemone \
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
	turmite \
	uvlight \
	uvmap \
	uvmap-rgb \
	weave \
	wireframe

FAILURES := \
	tests \
	vscaler

EXTRA-FILES += main.o
CLEAN-FILES += main.o

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
