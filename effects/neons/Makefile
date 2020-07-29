TOPDIR := $(realpath ../..)

CLEAN-FILES := \
            data/greet_ada.c \
	    data/greet_blacksun.c \
	    data/greet_dcs.c \
	    data/greet_dekadence.c \
	    data/greet_desire.c \
	    data/greet_dinx.c \
	    data/greet_elude.c \
	    data/greet_fd.c \
	    data/greet_floppy.c \
	    data/greet_lemon.c \
	    data/greet_loonies.c \
	    data/greet_moods.c \
	    data/greet_nah.c \
	    data/greet_rno.c \
	    data/greet_skarla.c \
	    data/greet_speccy.c \
	    data/greet_tulou.c \
	    data/greet_wanted.c \
	    data/greet_ycrew.c \
	    data/neons.c data/neons.png

PNG2C.greet_ada := --bitmap greet_ada,32x84x3
PNG2C.greet_blacksun  := --bitmap greet_blacksun,32x104x3
PNG2C.greet_dcs := --bitmap greet_dcs,48x118x3
PNG2C.greet_dekadence := --bitmap greet_dekadence,32x120x3
PNG2C.greet_desire := --bitmap greet_desire,32x88x3
PNG2C.greet_dinx := --bitmap greet_dinx,32x110x3
PNG2C.greet_elude := --bitmap greet_elude,32x120x3
PNG2C.greet_fd := --bitmap greet_fd,64x120x3
PNG2C.greet_floppy := --bitmap greet_floppy,32x115x3
PNG2C.greet_lemon := --bitmap greet_lemon,48x120x3
PNG2C.greet_loonies := --bitmap greet_loonies,32x107x3
PNG2C.greet_moods := --bitmap greet_moods,48x120x3 --palette moods_pal,8
PNG2C.greet_nah := --bitmap greet_nah,32x98x3
PNG2C.greet_rno := --bitmap greet_rno,32x84x3 --palette rno_pal,8
PNG2C.greet_skarla := --bitmap greet_skarla,48x120x3
PNG2C.greet_speccy := --bitmap greet_speccy,32x119x3
PNG2C.greet_tulou := --bitmap greet_tulou,32x120x3
PNG2C.greet_wanted := --bitmap greet_wanted,48x120x3
PNG2C.greet_ycrew := --bitmap greet_ycrew,32x120x3
PNG2C.neons := --bitmap background,320x256x4 --palette background_pal,16

include $(TOPDIR)/build/effect.mk

%.png: %.py
	@echo "[GEN] $@"
	$(PYTHON3) $< $@
