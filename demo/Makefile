TOPDIR = $(realpath $(CURDIR)/..)

OBJS := demo.o spy-shs10.o
BINS := spy-shs10

all:: $(BINS)

spy-shs10: demo.o spy-shs10.o libaudio.a libdistort.a libgfx.a libsystem.a libstd.a

spy-shs10.o: CFLAGS += -Wno-unused

archive: spy-shs10
	lha a $^.lha $^ data

clean::
	@$(RM) -v $(BINS) *.lha

include $(TOPDIR)/Makefile.common

.PHONY: archive

# vim: sw=8 ts=8
