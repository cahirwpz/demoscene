TOPDIR = $(realpath $(CURDIR)/..)

OBJS = audio.o c2p1x1_8_c5_bm.o check.o display.gfx.o fileio.o hardware.o \
       iff.o inflate.o input.o rwops.o rwops-file.o rwops-memory.o \
       vblank.o zip.o

libsystem.a: $(OBJS)

include $(TOPDIR)/Makefile.common
