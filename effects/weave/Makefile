TOPDIR := $(realpath ../..)

CLEAN-FILES := data/bar.c data/stripes.c data/stripe-up.c data/stripe-down.c \
	       data/stripe-up-2.c data/stripe-down-2.c

PNG2C.bar := --bitmap bar,480x33x4 --palette bar_pal,16
PNG2C.stripes := --sprite stripes,256,4
PNG2C.stripe-up := --palette stripe_up_pal,4
PNG2C.stripe-down := --palette stripe_down_pal,4
PNG2C.stripe-up-2 := --palette stripe_up_2_pal,4
PNG2C.stripe-down-2 := --palette stripe_down_2_pal,4

SOURCES := weave.c

include $(TOPDIR)/build/effect.mk
