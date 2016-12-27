CPPFLAGS = -Iinclude
SUBDIRS = ahx p61 sys effects

BINS = ExitUAE 

all: crt0.o $(SUBDIRS) $(BINS)

$(SUBDIRS):
	make -C $@

ExitUAE: ExitUAE.o

clean-libs:
	@for subdir in $(SUBDIRS); do $(MAKE) -C $$subdir clean; done

clean::	clean-libs
	rm -f *.7z

ctags:
	ctags -R .

cscope:
	cscope -b sys/*.[ch]

include Makefile.common

.PHONY:	$(SUBDIRS) cscope tags clean clean-libs
