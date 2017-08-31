IMAGES := $(subst .png,.ilbm,$(wildcard *.png))

all: $(IMAGES)

include ../../build.mk

clean::
	$(RM) $(IMAGES)
