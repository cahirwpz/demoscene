TOPDIR = $(CURDIR)/..

include $(TOPDIR)/Makefile.common

OBJS = atompool.o node.o list.o memory.o resource.o stack.o tree.o

libstd.a: $(OBJS)

atompool.o: atompool.c atompool.h memory.h types.h
node.o: node.c node.h types.h
list.o: list.c list.h node.h types.h
memory.o: memory.c memory.h types.h
resource.o: resource.c resource.h types.h
stack.o: stack.c stack.h list.h types.h
tree.o: tree.c tree.h list.h types.h

clean:
	$(RM) *.a *.o *~

.PHONY: clean
