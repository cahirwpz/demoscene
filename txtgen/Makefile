TOPDIR = $(CURDIR)/..

include $(TOPDIR)/Makefile.common

OBJS = plasma.o perlin.o perlinplasma.o

libtxtgen.a: $(OBJS)

plasma.o: plasma.c
perlin.o: perlin.c
perlinplasma.o: perlinplasma.c

clean:
	$(RM) *.a *.o *~

.PHONY: clean
