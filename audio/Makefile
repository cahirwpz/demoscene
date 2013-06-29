TOPDIR = $(realpath $(CURDIR)/..)

OBJS = ahibase.o stream.o adpcm.o wave.o

all:: libaudio.a test-wave

test-wave: LDFLAGS += -L$(TOPDIR)/system -L$(TOPDIR)/std
test-wave: LDLIBS += -lsystem -lstd
test-wave: test-wave.o wave.o
libaudio.a: $(OBJS)

clean::
	rm -f test-wave

include $(TOPDIR)/Makefile.common
