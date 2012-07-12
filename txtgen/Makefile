TOPDIR = $(CURDIR)/..

include $(TOPDIR)/Makefile.common

OBJS = channel.o convolution2d.o plasma.o perlin.o perlinplasma.o txtgen.o

libtxtgen.a: $(OBJS)

channel.o: channel.c
convolution2d.o: convolution2d.c convolution2d.h
plasma.o: plasma.c
perlin.o: perlin.c
perlinplasma.o: perlinplasma.c
txtgen.o: txtgen.c txtgen.h

clean:
	$(RM) *.a *.o *~

.PHONY: clean
