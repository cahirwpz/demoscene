TOPDIR = $(CURDIR)/..

include $(TOPDIR)/Makefile.common

OBJS = plasma.o perlin.o

libtxtgen.a: $(OBJS)

plasma.o: plasma.c
perlin.o: perlin.c

clean:
	$(RM) *.a *.o *~

.PHONY: clean
