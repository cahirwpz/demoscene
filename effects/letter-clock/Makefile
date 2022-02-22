TOPDIR := $(realpath ../..)

CLEAN-FILES := data/clock-bg.c data/clock-palette.c data/movement_data.c

PNG2C.clock-bg := --bitmap clock_bg,320x256x3

$(echo "[CIRCLES] clock-palette.c")
data/clock-palette.c:
	python3 data/palette_extend.py --max_colours 32 --name clock_pal --image_path data/clock-bg.png --extended_palette data/palette.txt > data/clock-palette.c

$(echo "[CIRCLES] movement_data.c")
data/movement_data.c:
	python3 data/gen_sin_cos.py 256 75 > data/movement_data.c

include $(TOPDIR)/build/effect.mk
