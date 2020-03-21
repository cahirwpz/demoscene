TOPDIR := $(realpath ../..)

SOURCES_GEN := data/blurred-pal-1.c data/blurred-pal-2.c
DATA_GEN := data/blurred-clip.ilbm

include $(TOPDIR)/build.effect.mk

data/blurred-pal-1.c: PNG2C += --palette blurred_1_pal,32
data/blurred-pal-2.c: PNG2C += --palette blurred_2_pal,32
