CPPFLAGS = -Iinclude
SUBDIRS = $(wildcard lib*) effects

BINS = ExitUAE bootloader.bin

all: crt0.o $(BINS) $(SUBDIRS)

$(SUBDIRS):
	make -C $@

ExitUAE: ExitUAE.o
bootloader.bin: ASFLAGS += -phxass

clean-libs:
	@for subdir in $(SUBDIRS); do $(MAKE) -C $$subdir clean; done

clean::	clean-libs
	rm -vf bootloader.bin

ctags:
	ctags -R .

cscope:
	cscope -b sys/*.[ch]

include Makefile.common

.PHONY:	$(SUBDIRS) cscope tags clean clean-libs
