TOPDIR = $(CURDIR)/..

include $(TOPDIR)/Makefile.common

OBJS = map.o render.o render_opt.o tunnel.o

libdistortions.a: $(OBJS)

map.o: map.c map.h
render.o: render.c map.h
render_opt.o: render_opt.s map.h
tunnel.o: tunnel.c map.h

clean:
	$(RM) *.a *.o *~

.PHONY: clean
