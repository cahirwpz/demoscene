SOURCES_ASM := \
	bootloader.s \
	crt0.s

SUBDIRS := \
	libc \
	lib2d \
	lib3d \
	libahx \
	libp61 \
	libsync \
	libsys

all: crt0.o bootloader.bin ctags cscope $(foreach dir,$(SUBDIRS),build-$(dir))

include ../build.mk

bootloader.bin: ASFLAGS += -phxass

FILES := $(shell find $(SUBDIRS) include -iname '*.c' -or -iname '*.h')

ctags:
	ctags -R $(FILES)

cscope:
	cscope -b $(FILES)

clean::	$(foreach dir,$(SUBDIRS),clean-$(dir))
	$(RM) crt0.o bootloader.bin
	$(RM) tags cscope.out

.PHONY:	cscope ctags
