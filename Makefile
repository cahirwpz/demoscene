TOPDIR = $(realpath .)

all: build-base build-effects

include $(TOPDIR)/build.mk

clean:: clean-effects clean-base
