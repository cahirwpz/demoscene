TOPDIR = $(realpath $(CURDIR)/..)

OBJS = 8svx.o blitter.o common.o circle.o console.o coplist.o ffp.o gfx.o \
       hardware.o iff.o ilbm.o io-dos.o io-extra.o interrupts.o keyboard.o \
       line.o memory.o mouse.o pixmap.o qsort.o random.o reader.o sound.o \
       sprite.o tga.o lowlevel.o 2d.o fx.o sintab.o color.o lzo.o inflate.o \
       timeline.o sync.o sync-load.o traphandler.o sort.o amigahunk.o \
       3d-transform.o 3d-mesh.o 3d-object.o 3d-clip.o 3d-lwo.o \
       bltop-add-sat.o bltop-copy.o bltop-clear.o bltop-dec-sat.o \
       bltop-inc-sat.o bltop-mask.o context.o newstackswap.o \
       png.o c2p_1x1_4.o \
       __mulsi3.o __divsi3.o __udivsi3.o __modsi3.o __umodsi3.o

all:	libsys.a bootloader.bin

libsys.a: $(OBJS)

bootloader.bin: bootloader.s

sintab.s: sintab.py
	python sintab.py

include $(TOPDIR)/Makefile.common

# vim: sw=8 ts=8
