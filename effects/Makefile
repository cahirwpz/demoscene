TOPDIR := $(realpath ..)

SUBDIRS := \
	abduction \
	anim \
	anim-polygons \
	ball \
	bobs3d \
	blurred \
	blurred3d \
	bumpmap-rgb \
	butterfly-gears \
	cathedral \
	carrion \
	circles \
	color-cycling \
	credits \
	darkroom \
	dna3d \
	empty \
	flatshade \
	flatshade-convex \
	floor \
	floor-old \
	fire-rgb \
	forest \
	game-of-life \
	glitch \
	glitches \
	growing-tree \
	gui \
	highway \
	kbtest \
	layers \
	lines \
	loader \
	magnifying-glass \
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
	roller \
	rotator \
	sea-anemone \
	shapes \
	showpchg \
	spooky-tree \
	stencil3d \
	stripes \
	textscroll \
	thunders \
	texobj \
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
