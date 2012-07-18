TOPDIR = $(CURDIR)/..

include $(TOPDIR)/Makefile.common

OBJS = common.o map.o offset.o render.o render_opt.o sine.o tunnel.o twirl.o

libdistort.a: $(OBJS)

common.o: common.c common.h
map.o: map.c common.h generate.h
offset.o: offset.c common.h generate.h
render.o: render.c common.h
render_opt.o: render_opt.s common.h
sine.o: sine.c common.h generate.h
tunnel.o: tunnel.c common.h generate.h
twirl.o: twirl.c common.h generate.h

clean:
	$(RM) *.a *.o *~

.PHONY: clean
