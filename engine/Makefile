TOPDIR = $(CURDIR)/..

include $(TOPDIR)/Makefile.common

OBJS = object.o

libengine.a: $(OBJS)

object.o: object.c object.h

clean:
	$(RM) *.a *.o *~

.PHONY: clean
