TOPDIR := $(realpath ..)

LIBNAME := loader
SOURCES := \
	autoinit.c \
	effect.c \
	boot.c \
	os.c \
	sync.c \
	profiler.c \
	main.c \
	drivers/cia-frame.c \
	drivers/cia-icr.c \
	drivers/cia-line.c \
	drivers/cia-timer.c \
	drivers/event.c \
	drivers/floppy.c \
	drivers/io.c \
	drivers/io-extra.c \
	drivers/keyboard.c \
	drivers/mouse.c \
	drivers/serial.c \
	kernel/amigahunk.c \
	kernel/interrupt.c \
	kernel/memory.c

BUILD-FILES = crt0.o
		
include $(TOPDIR)/build/lib.mk
