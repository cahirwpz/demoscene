TOPDIR = $(CURDIR)/..

include $(TOPDIR)/Makefile.common

OBJS = atompool.o memory.o resource.o slist.o stack.o

libstd.a: $(OBJS)

atompool.o: atompool.c atompool.h memory.h types.h
memory.o: memory.c memory.h types.h
resource.o: resource.c resource.h resource_internal.h
slist.o: slist.c slist.h types.h
stack.o: stack.c stack.h slist.h types.h

clean:
	$(RM) *.a *.o *~

.PHONY: clean
