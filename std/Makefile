TOPDIR = $(CURDIR)/..

include $(TOPDIR)/Makefile.common

OBJS = slist.o stack.o

all: std.lib

slist.o: slist.c slist.h types.h
stack.o: stack.c stack.h slist.h types.h

std.lib: $(OBJS)

clean:
	$(RM) *.lib *.o *~

.PHONY: clean
