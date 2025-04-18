TOPDIR := $(realpath ../..)

CLEAN-FILES := data/car-left-2.c data/car-right-2.c \
	       data/city-bottom-2.c data/city-top-2.c \
	       data/lane.c data/sprite.c

PNG2C.lane := --bitmap lane_bg,320x40x4
PNG2C.city-top-2 := --bitmap city_top,320x70x4 --palette city_top,16
PNG2C.city-bottom-2 := --bitmap city_bottom,320x70x4 --palette city_bottom,16
PNG2C.car-left-2 := --bitmap car_left,32x10x4 --palette car_left,16
PNG2C.car-right-2 := --bitmap car_right,32x10x4 --palette car_right,16
PNG2C.sprite := --sprite sprite,24,8 --palette sprite,4

include $(TOPDIR)/build/effect.mk
