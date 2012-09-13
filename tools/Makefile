TOPDIR = $(CURDIR)/..

include $(TOPDIR)/Makefile.common

OBJS = curves.o frame.o gradient.o loopevent.o

libtools.a: $(OBJS)

curves.o: curves.c curves.h
frame.o: frame.c frame.h
gradient.o: gradient.c gradient.h
loopevent.o: loopevent.c loopevent.h

clean:
	$(RM) *.a *.o *~

.PHONY: clean
