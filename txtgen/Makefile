TOPDIR = $(CURDIR)/..

include $(TOPDIR)/Makefile.common

OBJS = channel.o plasma.o perlin.o perlinplasma.o txtgen.o

libtxtgen.a: $(OBJS)

channel.o: channel.c
plasma.o: plasma.c
perlin.o: perlin.c
perlinplasma.o: perlinplasma.c
txtgen.o: txtgen.c txtgen.h

clean:
	$(RM) *.a *.o *~

.PHONY: clean
