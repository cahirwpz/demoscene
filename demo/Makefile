TOPDIR = $(CURDIR)/..

include $(TOPDIR)/Makefile.common

LIBS := $(LIBDIR) -lgfx -ltools -lsystem -lstd $(LIBS)

all: demo.o

demo.o: demo.c demo.h

clean:
	$(RM) *~ *.o $(BINS)

.PHONY: all archive $(BINS)

# vim: sw=8 ts=8
