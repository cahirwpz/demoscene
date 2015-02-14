TOPDIR = $(realpath $(CURDIR)/..)

BINS := spy-shs10 wpz-wcca
OBJS := spy-shs10.o wpz-wcca.o config.o timeline.o demo.o envelope.o
LIBS := libaudio.a libengine.a libuvmap.a libjson.a libtools.a libgfx.a \
	libsystem.a libstd.a 

all:: spy-shs10.syms wpz-wcca.syms $(BINS)

spy-shs10: spy-shs10.o config.o timeline.o demo.o envelope.o $(LIBS)

wpz-wcca: wpz-wcca.o config.o timeline.o demo.o envelope.o $(LIBS)

spy-shs10.o:: CFLAGS += -Wno-unused
wpz-wcca.o:: CFLAGS += -Wno-unused
demo.o:: WFLAGS += -Wno-error

%.syms: %.c gen-bindings.py
	@echo "SYMS $(subst $(TOPDIR)/,,$(realpath $<))"
	@./gen-bindings.py $< >$@

archive: wpz-wcca
	rm -f $^.7z
	7z a $^.7z $^ $^.json data/*.{8,pal,robj} 

clean::
	@$(RM) -v $(BINS) *.lha *.syms

include $(TOPDIR)/Makefile.common

.PHONY: all archive

# vim: sw=8 ts=8
