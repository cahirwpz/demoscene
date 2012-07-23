TOPDIR = $(CURDIR)/..

include $(TOPDIR)/Makefile.common

OBJS = matrix3d.o mesh.o ms3d.o object.o plane.o scene.o sphere.o vector3d.o

libengine.a: $(OBJS)

matrix3d.o: matrix3d.c matrix3d.h
mesh.o: mesh.c mesh.h
ms3d.o: ms3d.c ms3d.h matrix3d.h
object.o: object.c object.h mesh.h
plane.o: plane.c plane.h
scene.o: scene.c scene.h object.h mesh.h
sphere.o: sphere.c sphere.h
vector3d.o: vector3d.c vector3d.h

clean:
	$(RM) *.a *.o *~

.PHONY: clean
