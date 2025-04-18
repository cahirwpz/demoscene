TOPDIR := $(realpath ..)

CPPFLAGS += -D_SYSTEM

LIBNAME := system
SOURCES := \
	amigaos.c \
	autoinit.c \
	debug.c \
	dputchar.S \
	kickfix.S \
	kputchar.S \
	effect.c \
	loader.c \
	profiler.c \
	trackmo.c \
	vblank.c \
	drivers/cia-frame.c \
	drivers/cia-icr.c \
	drivers/cia-line.c \
	drivers/cia-timer.c \
	drivers/event.c \
	drivers/floppy.c \
	drivers/filesys.c \
	drivers/keyboard.c \
	drivers/memfile.c \
	drivers/mouse.c \
	drivers/serial.c \
	kernel/amigahunk.c \
	kernel/cpu.S \
	kernel/exception.c \
	kernel/file.c \
	kernel/interrupt.c \
	kernel/intr-entry.S \
	kernel/memory.c \
	kernel/mutex.c \
	kernel/task.c \
	kernel/trap-entry.S \
	kernel/trap.c 

CFLAGS.amigaos = -Wno-strict-prototypes

BUILD-FILES = crt0.o
		
include $(TOPDIR)/build/lib.mk
