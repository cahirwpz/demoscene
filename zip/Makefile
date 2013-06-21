TOPDIR = $(realpath $(CURDIR)/..)

OBJS := ziplist.o
BINS := ziplist

all:: $(BINS)

ziplist: ziplist.o libtinf.a

libtinf.a:
	make -C tinf

clean::
	@$(RM) -v $(BINS)
	make -C tinf clean

include $(TOPDIR)/Makefile.common

CPPFLAGS += -Itinf
LDFLAGS += -Ltinf
