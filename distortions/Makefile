TOPDIR = $(CURDIR)/..

include $(TOPDIR)/Makefile.common

OBJS = map.o render.o render_opt.o sine.o tunnel.o

libdistortions.a: $(OBJS)

map.o: map.c map.h
render.o: render.c map.h
render_opt.o: render_opt.s map.h
sine.o: sine.c map.h generate.h
tunnel.o: tunnel.c map.h generate.h

clean:
	$(RM) *.a *.o *~

.PHONY: clean
