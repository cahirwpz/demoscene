TOPDIR := $(realpath ../..)

CLEAN-FILES := data/clock-bg.c data/movement_data.c

# In this example don't want to loose the handcrafted palette
# although this bit probably can be done nicer
PNG2C.clock-bg := --bitmap clock_bg,320x256x3 \
				# --palette clock_pal,8

$(echo "[CIRCLES] movement_data.c")
data/movement_data.c:
	python3 data/gen_sin_cos.py 256 75 > data/movement_data.c

include $(TOPDIR)/build/effect.mk
