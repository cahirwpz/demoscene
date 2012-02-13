TOPDIR = $(CURDIR)/..

include $(TOPDIR)/Makefile.common

OBJS = object.o

all: engine.lib

object.o: object.c object.h

engine.lib: $(OBJS)

clean:
	$(RM) *.lib *.o *~

.PHONY: clean
