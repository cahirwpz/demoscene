TOPDIR := $(realpath ../..)

SOURCES := pt.asm

include $(TOPDIR)/build/lib.mk

VASMFLAGS.pt += -DMINIMAL=0 -DENABLE_SAWRECT=0 -DENABLE_VUMETER=0 -DJAZZCAT=1

pt.o: pt.asm ptplayer.asm $(TOPDIR)/include/ptplayer.h
