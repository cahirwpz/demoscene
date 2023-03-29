TOPDIR := $(realpath ../..)

SOURCES := pt.asm

include $(TOPDIR)/build/lib.mk

VASMFLAGS.pt += -DMINIMAL=1 -DENABLE_SAWRECT=0 -DENABLE_VUMETER=0

pt.o: pt.asm ptplayer.asm $(TOPDIR)/include/ptplayer.h
