TOPDIR = $(CURDIR)/..

include $(TOPDIR)/Makefile.common

OBJS = distortion.o distortion_opt.o

libdistortions.a: $(OBJS)

distortion.o: distortion.c distortion.h
distortion_opt.o: distortion_opt.s distortion.h

clean:
	$(RM) *.a *.o *~

.PHONY: clean
