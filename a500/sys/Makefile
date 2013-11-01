TOPDIR = $(realpath $(CURDIR)/..)

OBJS = 8svx.o blitter.o coplist.o file.o gfx.o hardware.o iff.o ilbm.o print.o \
       sound.o sprite.o vbr.o memory.o \
       __mulsi3.o __divsi3.o __udivsi3.o __modsi3.o __umodsi3.o

libsys.a: $(OBJS)

include $(TOPDIR)/Makefile.common

# vim: sw=8 ts=8
