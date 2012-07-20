TOPDIR = $(CURDIR)/..

include $(TOPDIR)/Makefile.common

OBJS = curves.o frame.o

libtools.a: $(OBJS)

curves.o: curves.c curves.h
frame.o: frame.c frame.h

clean:
	$(RM) *.a *.o *~

.PHONY: clean
