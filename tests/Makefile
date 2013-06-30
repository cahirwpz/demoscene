TOPDIR = $(realpath $(CURDIR)/..)

BINS := json wave-file
LIBS := libsystem.a libstd.a

all:: $(BINS)

json: json.o libjson.a $(LIBS)
wave-file: wave-file.o libaudio.a $(LIBS)

archive:
	7z a "bins-$$(date +%F).7z" $(BINS) data

clean::
	@$(RM) -v $(BINS)

include $(TOPDIR)/Makefile.common

.PHONY: all archive

# vim: sw=8 ts=8
