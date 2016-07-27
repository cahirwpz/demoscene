TOPDIR = $(realpath $(CURDIR)/..)

OBJS = 8svx.o blitter.o common.o circle.o console.o coplist.o ffp.o gfx.o \
       hardware.o iff.o ilbm.o inflate.o io-dos.o io-extra.o keyboard.o \
       line.o memory.o mouse.o pixmap.o qsort.o random.o reader.o sound.o \
       sprite.o tga.o fx.o sintab.o color.o newstackswap.o hrtmon.o \
       timeline.o sync.o sync-load.o sort.o amigahunk.o png.o c2p_1x1_4.o \
       3d-transform.o 3d-mesh.o 3d-object.o 3d-clip.o 3d-lwo.o interrupts.o \
       bltop-add-sat.o bltop-copy.o bltop-clear.o bltop-dec-sat.o serial.o \
       bltop-inc-sat.o bltop-mask.o rawio-ser.o rawio-par.o io-floppy.o \
       __mulsi3.o __divsi3.o __udivsi3.o __modsi3.o __umodsi3.o floppy.o \
       effect.o event.o tasks.o gui.o 2d.o 2d-clip.o

all:	libsys.a bootloader.bin

libsys.a: $(OBJS)

sintab.s: sintab.py
	python sintab.py

bootloader.bin: ASFLAGS += -phxass

include $(TOPDIR)/Makefile.common

clean::
	rm -vf bootloader.bin

# vim: sw=8 ts=8
