MODULE := JazzCat-Automatic

all: $(MODULE).ctr

include ../../build.mk

%.ctr: %.mod
	$(PYTHON) CinterConvert.py $^ $@ $(@:.ctr=.smp)

clean::
	$(RM) $(MODULE).ctr $(MODULE).smp
