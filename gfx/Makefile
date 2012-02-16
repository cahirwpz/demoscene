TOPDIR = $(CURDIR)/..

include $(TOPDIR)/Makefile.common

OBJS = blit.o canvas.o ellipse.o hsl.o line.o matrix2d.o matrix3d.o palette.o \
			 pixbuf.o transformations.o triangle.o vector3d.o

all: gfx.lib

blit.o: blit.c blit.h pixbuf.h
canvas.o: canvas.c canvas.h
ellipse.o: ellipse.c ellipse.h canvas.h
hsl.o: hsl.c hsl.h
line.o: line.c line.h
matrix2d.o: matrix2d.c matrix2d.h
matrix3d.o: matrix3d.c matrix3d.h
palette.o: palette.c palette.h common.h
pixbuf.o: pixbuf.c pixbuf.h
transformations.o: transformations.c transformations.h matrix2d.h matrix3d.h
triangle.o: triangle.c triangle.h canvas.h
vector3d.o: vector3d.c vector3d.h

gfx.lib: $(OBJS)

clean:
	$(RM) *.lib *.o *~

.PHONY: clean
