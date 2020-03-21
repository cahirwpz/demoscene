TOPDIR := $(realpath ../..)

SOURCES_GEN := data/flares32.c
DATA_GEN := data/pilka.3d

LIBS := lib3d

include $(TOPDIR)/build.effect.mk

data/flares32.c: PNG2C += --bitmap bobs,512x32x3,+interleaved \
	                  --palette bobs_pal,8
