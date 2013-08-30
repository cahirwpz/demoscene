TOPDIR = $(realpath $(CURDIR)/..)

OBJS := config.o demo.o spy-shs10.o
BINS := spy-shs10

all:: $(BINS)

spy-shs10: $(OBJS) libaudio.a libuvmap.a libjson.a libtools.a libgfx.a \
	libsystem.a libstd.a 

spy-shs10.o: CFLAGS += -Wno-unused
spy-shs10: LIBS += -lauto

archive: spy-shs10
	lha a $^.lha $^ data

clean::
	@$(RM) -v $(BINS) *.lha

include $(TOPDIR)/Makefile.common

.PHONY: archive

# vim: sw=8 ts=8
