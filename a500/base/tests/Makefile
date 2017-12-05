BINS := \
	parseiff.exe \
	parselwo.exe \
	parsepng.exe \
	play8svx.exe

all: $(BINS)

play8svx.exe: LIBS = $(TOPDIR)/base/libaudio/libaudio.a

include ../../build.mk

%.exe: %.o 
	@echo "[$(DIR):ld] $@"
	$(CC) -o $@ $^ $(LIBS) $(TOPDIR)/base/libsys/libsys.a $(LDLIBS)
