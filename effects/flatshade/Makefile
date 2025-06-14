TOPDIR := $(realpath ../..)

CLEAN-FILES := data/flatshade-pal.c data/codi.c data/pilka.c \
	data/stripe-?.c

LIBS := lib3d

PNG2C.flatshade-pal := --palette flatshade,16
PNG2C.stripe-1 := --sprite stripe_1,288,2 --palette stripe_1,4
PNG2C.stripe-2 := --sprite stripe_2,284,2 --palette stripe_2,4
PNG2C.stripe-3 := --sprite stripe_3,280,2 --palette stripe_3,4
PNG2C.stripe-4 := --sprite stripe_4,276,2 --palette stripe_4,4
OBJ2C.codi := --scale 488.0 --mesh-name codi

include $(TOPDIR)/build/effect.mk
