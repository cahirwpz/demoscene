TOPDIR := $(realpath ../..)

SOURCES := texobj.c mainloop.asm
CLEAN-FILES := data/cube-bg.c data/cube-tex?.c data/cube.c

LIBS := lib3d

PNG2C.cube-bg := --pixmap background,128x128x8,+displayable,+onlydata --palette texture,16
PNG2C.cube-tex0 := --pixmap texture_0,128x128x8,+onlydata
PNG2C.cube-tex1 := --pixmap texture_1,128x128x8,+onlydata
PNG2C.cube-tex2 := --pixmap texture_2,128x128x8,+onlydata
PNG2C.cube-tex3 := --pixmap texture_3,128x128x8,+onlydata
PNG2C.cube-tex4 := --pixmap texture_4,128x128x8,+onlydata
OBJ2C.cube := --scale 48.0 --mesh-name cube --no-edges --textured --texture-scale 2048.0

include $(TOPDIR)/build/effect.mk
