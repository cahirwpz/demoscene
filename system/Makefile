TOPDIR := $(realpath ..)

# AMIGAOS => save & restore AmigaOS context
#            (for intros or trackmos that can be launched from AmigaOS)
# TRACKMO => initialize file system and floppy device driver
CPPFLAGS += -DTRACKMO -D_SYSTEM

LIBNAME := system
SOURCES := \
	amigaos.c \
	autoinit.c \
	debug.c \
	debug-putchar.S \
	effect.c \
	jumptab.S \
	loader.c \
	main.c \
	profiler.c \
	syscall.S \
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

syscall.lds jumptab.S: system-api.py system-api.in 
	python3 system-api.py system-api.in syscall.lds jumptab.S

$(LDSCRIPT): syscall.lds
	touch $@

CLEAN-FILES += jumptab.S jumptab.o syscall.S syscall.o
