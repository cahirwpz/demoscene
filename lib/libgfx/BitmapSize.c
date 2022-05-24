#include <bitmap.h>

u_int BitmapSize(BitmapT *bitmap) {
  /* Allocate extra two bytes for scratchpad area.
   * Used by blitter line drawing. */
  return ((u_short)bitmap->bplSize * (u_short)bitmap->depth) + BM_EXTRA;
}
