TOPDIR = $(CURDIR)/..

include $(TOPDIR)/Makefile.common

OBJS = map.o offset.o render.o render_opt.o sine.o tunnel.o twirl.o

libdistortions.a: $(OBJS)

map.o: map.c map.h
offset.o: offset.c map.h generate.h
render.o: render.c map.h
render_opt.o: render_opt.s map.h
sine.o: sine.c map.h generate.h
tunnel.o: tunnel.c map.h generate.h
twirl.o: twirl.c map.h generate.h

clean:
	$(RM) *.a *.o *~

.PHONY: clean
