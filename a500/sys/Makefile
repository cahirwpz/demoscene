TOPDIR = $(realpath $(CURDIR)/..)

OBJS = 8svx.o blitter.o common.o circle.o console.o coplist.o ffp.o file.o \
       gfx.o hardware.o iff.o ilbm.o interrupts.o keyboard.o line.o memory.o \
       mouse.o pixmap.o print.o qsort.o random.o reader.o sound.o sprite.o \
       tga.o lowlevel.o 2d.o sintab.o colortab.o lzo.o inflate.o \
       timeline.o sync.o sync-load.o traphandler.o \
       3d-transform.o 3d-mesh.o 3d-object.o 3d-clip.o 3d-lwo.o \
       bltop-add-sat.o bltop-copy.o bltop-clear.o bltop-dec-sat.o bltop-inc-sat.o \
       __mulsi3.o __divsi3.o __udivsi3.o __modsi3.o __umodsi3.o

libsys.a: $(OBJS)

sintab.s: sintab.py
	python sintab.py

colortab.s: colortab.py
	python colortab.py

include $(TOPDIR)/Makefile.common

# vim: sw=8 ts=8
