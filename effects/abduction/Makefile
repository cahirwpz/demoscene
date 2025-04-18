TOPDIR := $(realpath ../..)

CLEAN-FILES := data/beam.c data/coq.c data/ring.c data/ufo.c data/bkg.c data/mid_beam.c data/side_beam_l.c data/side_beam_r.c

PNG2C.bkg  := --bitmap bkg,320x256x1
PNG2C.ufo  := --bitmap ufo,104x39x4
PNG2C.ring := --bitmap ring,56x16x3

PNG2C.coq         := --sprite coq,24,1
PNG2C.mid_beam    := --sprite mid_beam,200,1
PNG2C.side_beam_l := --sprite side_beam_l,200,1
PNG2C.side_beam_r := --sprite side_beam_r,200,1

include $(TOPDIR)/build/effect.mk
