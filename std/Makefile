TOPDIR = $(CURDIR)/..

include $(TOPDIR)/Makefile.common

OBJS = array.o atompool.o fastmath.o fp16.o node.o list.o math.o memory.o \
			 queue.o ptrarray.o resource.o stack.o tree.o

libstd.a: $(OBJS)

array.o: array.c array.h memory.h types.h
atompool.o: atompool.c atompool.h memory.h types.h
fastmath.o: fastmath.c fastmath.h math.h
fp16.o: fp16.c fp16.h math.h types.h
node.o: node.c node.h types.h
list.o: list.c list.h node.h types.h
math.o: math.c math.h types.h
memory.o: memory.c memory.h types.h
queue.o: queue.c memory.h types.h
ptrarray.o: ptrarray.c ptrarray.h types.h
resource.o: resource.c resource.h types.h
stack.o: stack.c stack.h memory.h types.h
tree.o: tree.c tree.h list.h types.h

clean:
	$(RM) *.a *.o *~

.PHONY: clean
