CPPFLAGS = -I../include
SUBDIRS = $(shell find . -mindepth 1 -maxdepth 1 -type d | sed 's,./,,')

all: startup.o $(SUBDIRS)

$(SUBDIRS):
	echo "MAKE $@"
	$(MAKE) -C $@

clean::
	@for d in $(SUBDIRS); do	\
	  $(MAKE) -C $$d clean;		\
	done

archive:
	7z a "a500-$$(date +%F-%H%M).7z" $(SUBDIRS)

.PHONY: all clean $(SUBDIRS)

include ../Makefile.common
