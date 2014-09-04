TOPDIR = $(realpath $(CURDIR)/..)

OBJS = 8svx.o blitter.o common.o circle.o console.o coplist.o file.o gfx.o \
       hardware.o iff.o ilbm.o interrupts.o keyboard.o line.o memory.o \
       mouse.o pixmap.o print.o reader.o sound.o sprite.o stop.o tga.o vbr.o \
       __mulsi3.o __divsi3.o __udivsi3.o __modsi3.o __umodsi3.o

libsys.a: $(OBJS)

include $(TOPDIR)/Makefile.common

# vim: sw=8 ts=8
