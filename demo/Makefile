TOPDIR = $(realpath $(CURDIR)/..)

BINS := spy-shs10 wpz-noname
OBJS := spy-shs10.o wpz-noname.o config.o timeline.o demo.o
LIBS := libaudio.a libuvmap.a libjson.a libtools.a libgfx.a libsystem.a \
	libstd.a 

all:: spy-shs10.syms wpz-noname.syms $(BINS)

spy-shs10: spy-shs10.o config.o timeline.o demo.o $(LIBS)

wpz-noname: wpz-noname.o config.o timeline.o demo.o $(LIBS)

spy-shs10.o:: CFLAGS += -Wno-unused
wzp-noname.o:: CFLAGS += -Wno-unused

%.syms: %.c gen-bindings.py
	@echo "SYMS $(subst $(TOPDIR)/,,$(realpath $<))"
	@./gen-bindings.py $< >$@

archive: spy-shs10
	lha a $^.lha $^ data

clean::
	@$(RM) -v $(BINS) *.lha *.syms

include $(TOPDIR)/Makefile.common

.PHONY: all archive

# vim: sw=8 ts=8
