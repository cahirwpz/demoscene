TOPDIR := $(realpath ../..)

CLEAN-FILES := data/bar.c data/stripe-up.c data/stripe-down.c

PNG2C.bar := --bitmap bar,480x33x4 --palette bar_pal,16
PNG2C.stripe-up := --sprite stripe_up,256,2,+array \
	           --palette stripe_up_pal,4
PNG2C.stripe-down := --sprite stripe_down,256,2,+array \
	             --palette stripe_down_pal,4
PNG2C.stripe-up-2 := --palette stripe_up_2_pal,4
PNG2C.stripe-down-2 := --palette stripe_down_2_pal,4

SOURCES := weave.c

include $(TOPDIR)/build/effect.mk
