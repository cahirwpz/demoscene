TOPDIR = $(CURDIR)/..

include $(TOPDIR)/Makefile.common

OBJS = common.o map.o offset.o render.o scaling.o sine.o tunnel.o twirl.o

libdistort.a: $(OBJS)

common.o: common.c common.h
map.o: map.c common.h generate.h
offset.o: offset.c common.h generate.h
render.o: render.c common.h
scaling.o: scaling.c scaling.h common.h
sine.o: sine.c common.h generate.h
tunnel.o: tunnel.c common.h generate.h
twirl.o: twirl.c common.h generate.h

clean:
	$(RM) *.a *.o *~

.PHONY: clean
