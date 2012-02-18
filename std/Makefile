TOPDIR = $(CURDIR)/..

include $(TOPDIR)/Makefile.common

OBJS = slist.o stack.o resource.o

libstd.a: $(OBJS)

slist.o: slist.c slist.h types.h
stack.o: stack.c stack.h slist.h types.h
resource.o: resource.c resource.h resource_internal.h

clean:
	$(RM) *.a *.o *~

.PHONY: clean
