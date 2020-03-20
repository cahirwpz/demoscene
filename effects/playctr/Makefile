TOPDIR := $(realpath ../..)

DATA_GEN := data/JazzCat-Automatic.ctr \
	    data/JazzCat-Automatic.smp

LIBS := libctr

include $(TOPDIR)/build.effect.mk

%.ctr %.smp: %.mod
	$(PYTHON3) data/CinterConvert.py $^ $@ $(@:.ctr=.smp)
