TOPDIR = $(CURDIR)/..

include $(TOPDIR)/Makefile.common

OBJS = plasma.o

libtxtgen.a: $(OBJS)

plasma.o: plasma.c

clean:
	$(RM) *.a *.o *~

.PHONY: clean
