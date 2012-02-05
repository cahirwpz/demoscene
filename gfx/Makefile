TOPDIR = $(CURDIR)/..

include $(TOPDIR)/Makefile.common

OBJS = bitmap.o canvas.o line.o matrix2d.o matrix3d.o transformations.o

all: gfx.lib

bitmap.o: bitmap.c bitmap.h 
canvas.o: canvas.c canvas.h
line.o: line.c line.h
matrix2d.o: matrix2d.c matrix2d.h
matrix3d.o: matrix3d.c matrix3d.h
transformations.o: transformations.c transformations.h matrix2d.h matrix3d.h

gfx.lib: $(OBJS)

clean:
	$(RM) *.lib *.o *~

.PHONY: clean
