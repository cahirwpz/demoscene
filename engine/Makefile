TOPDIR = $(CURDIR)/..

include $(TOPDIR)/Makefile.common

OBJS = mesh.o

libengine.a: $(OBJS)

mesh.o: mesh.c mesh.h

clean:
	$(RM) *.a *.o *~

.PHONY: clean
