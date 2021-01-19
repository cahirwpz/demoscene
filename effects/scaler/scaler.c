#include "effect.h"
#include "custom.h"
#include "copper.h"
#include "bitmap.h"
#include "palette.h"
#include "profiler.h"

#include "data/stars-die.c"

#define LINES 256
#define DEPTH 5

static CopListT *cp[2];
static short active = 0;

static void VerticalScalerForward(CopListT *cp, short ys, short height) {
  short rowmod = image.bytesPerRow * image.depth;
  int dy = (LINES << 16) / height;
  short n = (short)(dy >> 16) * (short)image.depth;
  short mod = (short)image.bytesPerRow * (short)(n - 1);
  int y = 0;
  short i;

  for (i = 1; i < height; i++) {
    short _mod = mod;
    int ny = y + dy;
    if ((u_short)ny < (u_short)y)
      _mod += rowmod;
    CopWaitSafe(cp, Y(ys + i), X(0));
    CopMove16(cp, bpl1mod, _mod);
    CopMove16(cp, bpl2mod, _mod);
    y = ny;
  }
}

static void CopSetupBitplanesReverse(CopListT *list, CopInsT **bplptr,
                                     const BitmapT *bitmap, u_short depth)
{
  short bytesPerLine = bitmap->bytesPerRow;
  int start;

  if (bitmap->flags & BM_INTERLEAVED)
    bytesPerLine *= (short)bitmap->depth;

  start = bytesPerLine * (short)(bitmap->height - 1);

  {
    void **planes = bitmap->planes;
    short n = depth - 1;
    short reg = CSREG(bplpt);

    do {
      CopInsT *ins = CopMoveLong(list, reg, (int)(*planes++) + start);

      if (bplptr)
        *bplptr++ = ins;

      reg += 4;
    } while (--n != -1);
  }

  {
    short modulo;

    if (bitmap->flags & BM_INTERLEAVED)
      modulo = (short)bitmap->bytesPerRow * (short)(depth + 1);
    else
      modulo = 2 * bitmap->bytesPerRow;

    CopMove16(list, bpl1mod, -modulo);
    CopMove16(list, bpl2mod, -modulo);
  }
}

static void VerticalScalerReverse(CopListT *cp, short ys, short height) {
  short rowmod = image.bytesPerRow * image.depth;
  int dy = (LINES << 16) / height;
  short n = (short)(dy >> 16) * (short)image.depth;
  short mod = (short)image.bytesPerRow * (short)(n + 1);
  int y = 0;
  short i;

  for (i = 1; i < height; i++) {
    short _mod = mod;
    int ny = y + dy;
    if ((u_short)ny < (u_short)y)
      _mod += rowmod;
    CopWaitSafe(cp, Y(ys + i), X(0));
    CopMove16(cp, bpl1mod, -_mod);
    CopMove16(cp, bpl2mod, -_mod);
    y = ny;
  }
}

static void MakeCopperList(CopListT *cp, short height) {
  short ys = (LINES - abs(height)) / 2;

  CopInit(cp);
  CopSetupDisplayWindow(cp, MODE_LORES, X(0), Y(ys), image.width, abs(height));
  if (height > 0) {
    CopSetupBitplanes(cp, NULL, &image, image.depth);
    VerticalScalerForward(cp, ys, height);
  } else if (height < 0) {
    CopSetupBitplanesReverse(cp, NULL, &image, image.depth);
    VerticalScalerReverse(cp, ys, -height);
  }
  CopEnd(cp);
}

static void Init(void) {
  LoadPalette(&image_pal, 0);
  SetupPlayfield(MODE_LORES, image.depth,
                 X(0), Y(0), image.width, image.height);

  cp[0] = NewCopList(40 + LINES * 3);
  cp[1] = NewCopList(40 + LINES * 3);
  MakeCopperList(cp[active], LINES);
  CopListActivate(cp[active]);

  EnableDMA(DMAF_RASTER);
}

static void Kill(void) {
  DisableDMA(DMAF_COPPER | DMAF_RASTER);
  DeleteCopList(cp[0]);
  DeleteCopList(cp[1]);
}

PROFILE(Scaler);

static void Render(void) {
  static short val = LINES, dir = -1;

  ProfilerStart(Scaler);
  MakeCopperList(cp[active], val);
  ProfilerStop(Scaler);

  CopListRun(cp[active]);
  TaskWaitVBlank();

  val += dir;
  if (val == -LINES)
    dir = -dir;

  active ^= 1;
}

EFFECT(scaler, NULL, NULL, Init, Kill, Render);
