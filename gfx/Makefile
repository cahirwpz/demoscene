TOPDIR = $(CURDIR)/..

include $(TOPDIR)/Makefile.common

OBJS = palette.o pixbuf.o canvas.o line.o matrix2d.o matrix3d.o transformations.o

all: gfx.lib

palette.o: palette.c palette.h common.h
pixbuf.o: pixbuf.c pixbuf.h
canvas.o: canvas.c canvas.h
line.o: line.c line.h
matrix2d.o: matrix2d.c matrix2d.h
matrix3d.o: matrix3d.c matrix3d.h
transformations.o: transformations.c transformations.h matrix2d.h matrix3d.h

gfx.lib: $(OBJS)

clean:
	$(RM) *.lib *.o *~

.PHONY: clean
