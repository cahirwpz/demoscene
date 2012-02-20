TOPDIR = $(CURDIR)/..

include $(TOPDIR)/Makefile.common

OBJS = blit.o canvas.o ellipse.o hsl.o line.o matrix2d.o matrix3d.o ms2d.o \
			 ms3d.o palette.o pixbuf.o ms2d.o triangle.o vector3d.o

libgfx.a: $(OBJS)

blit.o: blit.c blit.h pixbuf.h
canvas.o: canvas.c canvas.h
ellipse.o: ellipse.c ellipse.h canvas.h
hsl.o: hsl.c hsl.h
	$(CC) $(CFLAGS) -O0 -c -o $@ $<
line.o: line.c line.h
matrix2d.o: matrix2d.c matrix2d.h
matrix3d.o: matrix3d.c matrix3d.h
ms2d.o: ms2d.c ms2d.h matrix2d.h
ms3d.o: ms3d.c ms3d.h matrix3d.h
palette.o: palette.c palette.h common.h
pixbuf.o: pixbuf.c pixbuf.h
triangle.o: triangle.c triangle.h canvas.h
vector3d.o: vector3d.c vector3d.h

clean:
	$(RM) *.a *.o *~

.PHONY: clean
