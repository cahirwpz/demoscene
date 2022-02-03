TOPDIR := $(realpath ../..)

SOURCES := lines.c CpuLineOpt.asm CpuEdgeOpt.asm

include $(TOPDIR)/build/effect.mk

edge:
	gcc -O2 -Wall -o edge edge.c
	./edge

.PHONY: edge
