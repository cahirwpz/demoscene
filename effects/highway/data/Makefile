IMAGES := $(subst .png,.ilbm,$(wildcard highway-*.png))

all: $(IMAGES)

include ../../build.mk

clean::
	$(RM) $(IMAGES)
