TOPDIR = $(CURDIR)/..

include $(TOPDIR)/Makefile.common

OBJS = channel.o plasma.o perlin.o perlinplasma.o

libtxtgen.a: $(OBJS)

channel.o: channel.c
plasma.o: plasma.c
perlin.o: perlin.c
perlinplasma.o: perlinplasma.c

clean:
	$(RM) *.a *.o *~

.PHONY: clean
