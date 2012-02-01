TOPDIR = $(CURDIR)/..

include $(TOPDIR)/Makefile.common

OBJS = bitmap.o canvas.o line.o matrix3.o

all: gfx.lib

bitmap.o: bitmap.c bitmap.h 
canvas.o: canvas.c canvas.h
line.o: line.c line.h
matrix3.o: matrix3.c matrix3.h

gfx.lib: $(OBJS)

clean:
	$(RM) *.lib *.o *~

.PHONY: clean
