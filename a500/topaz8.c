#include <graphics/text.h>
#include <proto/graphics.h>

#include "blitter.h"
#include "coplist.h"
#include "print.h"

#define X(x) ((x) + 0x81)
#define Y(y) ((y) + 0x2c)

static BitmapT *screen;
static CopListT *cp;
static struct TextFont *topaz8;

void Load() {
  screen = NewBitmap(320, 256, 1, FALSE);
  cp = NewCopList(100);

  {
    struct TextAttr textattr = { "topaz.font", 8, FS_NORMAL, FPF_ROMFONT };
    topaz8 = OpenFont(&textattr);
  }
}

void Kill() {
  CloseFont(topaz8);
  DeleteCopList(cp);
  DeleteBitmap(screen);
}

static __regargs void BitmapPutChar(BitmapT *bitmap, UBYTE plane,
                                    UWORD x, UWORD y, char c)
{
  UBYTE *src = topaz8->tf_CharData;
  UBYTE *dst = screen->planes[plane];
  UWORD swidth = topaz8->tf_Modulo;
  UWORD dwidth = screen->width / 8;
  WORD h = 8;

  src += c - 32;
  dst += dwidth * y + (x >> 3);

  do {
    *dst = *src;
    src += swidth;
    dst += dwidth;
  } while (--h > 0);
}

void Main() {
  CopInit(cp);

  CopMove16(cp, bplcon0, BPLCON0_BPU(screen->depth) | BPLCON0_COLOR);
  CopMove16(cp, bplcon1, 0);
  CopMove16(cp, bplcon2, 0);
  CopMove32(cp, bplpt[0], screen->planes[0]);

  CopMove16(cp, ddfstrt, 0x38);
  CopMove16(cp, ddfstop, 0xd0);
  CopMakeDispWin(cp, X(0), Y(0), screen->width, screen->height);
  CopMove16(cp, color[0], 0x000);
  CopMove16(cp, color[1], 0xfff);

  CopEnd(cp);
  CopListActivate(cp);

  custom->dmacon = DMAF_SETCLR | DMAF_RASTER | DMAF_MASTER;

  BitmapPutChar(screen, 0, 40, 30, '0');

  WaitMouse();
}
