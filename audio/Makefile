TOPDIR = $(CURDIR)/..

include $(TOPDIR)/Makefile.common

OBJS = ahibase.o stream.o

libaudio.a: $(OBJS)

ahibase.o: ahibase.c
stream.o: stream.c stream.h

clean:
	$(RM) *.a *.o *~

.PHONY: clean
