TOPDIR = $(realpath $(CURDIR)/..)

BINS := exception json wave-file unzip readpng
LIBS := libsystem.a libstd.a

all:: $(BINS)

exception: exception.o $(LIBS)
json: json.o libjson.a $(LIBS)
wave-file: wave-file.o libaudio.a $(LIBS)
unzip: unzip.o $(LIBS) libtinf.a 
readpng: readpng.o libgfx.a libtinf.a $(LIBS)

archive:
	7z a "bins-$$(date +%F).7z" $(BINS) data

clean::
	@$(RM) -v $(BINS)

include $(TOPDIR)/Makefile.common

.PHONY: all archive

# vim: sw=8 ts=8
