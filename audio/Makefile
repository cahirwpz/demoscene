TOPDIR = $(realpath $(CURDIR)/..)

OBJS = ahibase.o stream.o adpcm.o wave.o

all:: libaudio.a test-wave

test-wave: test-wave.o wave.o
libaudio.a: $(OBJS)

clean::
	rm -f test-wave

include $(TOPDIR)/Makefile.common
