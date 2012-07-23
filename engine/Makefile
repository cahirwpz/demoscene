TOPDIR = $(CURDIR)/..

include $(TOPDIR)/Makefile.common

OBJS = mesh.o object.o plane.o scene.o sphere.o

libengine.a: $(OBJS)

mesh.o: mesh.c mesh.h
object.o: object.c object.h mesh.h
scene.o: scene.c scene.h object.h mesh.h
sphere.o: sphere.c sphere.h
plane.o: plane.c plane.h

clean:
	$(RM) *.a *.o *~

.PHONY: clean
