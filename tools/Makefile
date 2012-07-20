TOPDIR = $(CURDIR)/..

include $(TOPDIR)/Makefile.common

OBJS = curves.o

libtools.a: $(OBJS)

curves.o: curves.c curves.h

clean:
	$(RM) *.a *.o *~

.PHONY: clean
