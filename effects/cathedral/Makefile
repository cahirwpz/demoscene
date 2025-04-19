TOPDIR := $(realpath ../..)

CLEAN-FILES := data/cathedral-light.c data/cathedral-dark.c data/transition.c

PNG2C.cathedral-light := --bitmap cathedral_light,320x256x5 --palette cathedral,32
PNG2C.cathedral-dark := --bitmap cathedral_dark,320x256x5
PNG2C.transition := --bitmap transition,320x256x5

include $(TOPDIR)/build/effect.mk
