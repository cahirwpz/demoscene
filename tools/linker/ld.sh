#!/bin/sh

# -o weave.exe

TOPDIR=/Users/cahir/workspace/demoscene

./linker \
  $TOPDIR/system/crt0.o $TOPDIR/effects/weave/weave.o $TOPDIR/system/system.a \
  $TOPDIR/lib/libblit/libblit.a $TOPDIR/lib/libgfx/libgfx.a \
  $TOPDIR/lib/libmisc/libmisc.a $TOPDIR/lib/libc/libc.a
