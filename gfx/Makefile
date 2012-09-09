TOPDIR = $(CURDIR)/..

include $(TOPDIR)/Makefile.common

OBJS = blit.o canvas.o colors.o ellipse.o hsl.o line.o matrix2d.o ms2d.o \
			 palette.o pixbuf.o quantize.o spline.o triangle.o

libgfx.a: $(OBJS)

blit.o: blit.c blit.h pixbuf.h
canvas.o: canvas.c canvas.h
colors.o: colors.c colors.h
ellipse.o: ellipse.c ellipse.h canvas.h
hsl.o: hsl.c hsl.h
line.o: line.c line.h
matrix2d.o: matrix2d.c matrix2d.h
ms2d.o: ms2d.c ms2d.h matrix2d.h
palette.o: palette.c palette.h common.h
pixbuf.o: pixbuf.c pixbuf.h
quantize.o: quantize.c quantize.h pixbuf.h
spline.o: spline.c spline.h
triangle.o: triangle.c triangle.h canvas.h

clean:
	$(RM) *.a *.o *~

.PHONY: clean
