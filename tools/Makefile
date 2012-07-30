TOPDIR = $(CURDIR)/..

include $(TOPDIR)/Makefile.common

OBJS = curves.o frame.o loopevent.o

libtools.a: $(OBJS)

curves.o: curves.c curves.h
frame.o: frame.c frame.h
loopevent.o: loopevent.c loopevent.h

clean:
	$(RM) *.a *.o *~

.PHONY: clean
