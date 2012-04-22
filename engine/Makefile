TOPDIR = $(CURDIR)/..

include $(TOPDIR)/Makefile.common

OBJS = mesh.o object.o

libengine.a: $(OBJS)

mesh.o: mesh.c mesh.h
object.o: object.c object.h mesh.h

clean:
	$(RM) *.a *.o *~

.PHONY: clean
