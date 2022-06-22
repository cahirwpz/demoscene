TOPDIR := $(realpath ..)

# AMIGAOS => save & restore AmigaOS context
#            (for intros or trackmos that can be launched from AmigaOS)
AMIGAOS := 0

SOURCES := \
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

ifneq ($(AMIGAOS), 0)
CPPFLAGS += -DAMIGAOS
SOURCES += amigaos.c
endif

CPPFLAGS += -D_SYSTEM
LIBS += libc libmisc
LDEXTRA = $(foreach lib,$(LIBS),$(TOPDIR)/lib/$(lib)/$(lib).a)

all: system.exe

include $(TOPDIR)/build/common.mk

syscall.S jumptab.S: system-api.py system-api.in 
	@echo "[SYSCALL] system-api.in -> syscall.S jumptab.S"
	python3 system-api.py system-api.in syscall.S jumptab.S

# Check if library is up-to date if someone is asking explicitely
$(TOPDIR)/lib/lib%.a: FORCE
	$(MAKE) -C $(dir $@) $(notdir $@)

system.exe.dbg system.exe: crt0.o $(OBJECTS) $(LDEXTRA) $(LDSCRIPT)
	@echo "[LD] $(addprefix $(DIR),$(OBJECTS)) -> $(DIR)$@"
	$(LD) $(LDFLAGS) -L$(TOPDIR)/system -T$(LDSCRIPT) -Map=$@.map -o $@ \
		--start-group $(filter-out %.lds,$^) --end-group
	$(CP) $@ $@.dbg
	$(STRIP) $@

CLEAN-FILES += system.exe system.exe.dbg system.exe.map
CLEAN-FILES += jumptab.S jumptab.o syscall.S syscall.o
