TOPDIR := $(realpath ../..)

CLEAN-FILES := data/kurak-head.c data/background-data.* data/background-pal.*

LIBS := lib3d

PNG2C.background-data := --bitmap background,256x256x2
PNG2C.background-pal := --pixmap background_cols,4x256x12,+onlydata
PNG2C.pattern-1-1 := --bitmap pattern_1_1,256x256x2 --palette pattern_1,4
PNG2C.pattern-1-2 := --bitmap pattern_1_2,256x256x2
PNG2C.pattern-1-3 := --bitmap pattern_1_3,256x256x2
PNG2C.pattern-2-1 := --palette pattern_2,4
PNG2C.pattern-2-2 := 
PNG2C.pattern-2-3 := 
OBJ2C.flower := --scale 30.0 --mesh-name flower
OBJ2C.kurak-head := --scale 2.7 --mesh-name kurak

include $(TOPDIR)/build/effect.mk

%-data.png %-pal.png: %.png
	@echo "[SPLITIMG] $(DIR)$< -> $(DIR)$@"
	$(PYTHON3) $(TOPDIR)/effects/carrion/data/split-img.py 4 $<
