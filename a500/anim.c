#include "startup.h"
#include "2d.h"
#include "bltop.h"
#include "coplist.h"
#include "file.h"
#include "memory.h"
#include "ilbm.h"
#include "fx.h"

#define WIDTH  320
#define HEIGHT 240
#define DEPTH  5

static PaletteT *palette;
static BitmapT *screen;
static CopInsT *bplptr[DEPTH];
static CopListT *cp;
static WORD active = 0;

typedef struct {
  WORD width, height;
  WORD current, count;
  UBYTE *frame[0];
} AnimSpanT;

static AnimSpanT *anim;

static void Load() {
  screen = NewBitmap(WIDTH, HEIGHT, DEPTH + 1);
  palette = LoadPalette("data/boxes-pal.ilbm");
  anim = ReadFile("data/running.bin", MEMF_PUBLIC);

  {
    WORD i;

    for (i = 0; i < anim->count; i++)
      anim->frame[i] = (APTR)anim->frame[i] + (LONG)anim;
  }
}

static void UnLoad() {
  MemFreeAuto(anim);
  DeletePalette(palette);
  DeleteBitmap(screen);
}

static void Init() {
  custom->dmacon = DMAF_SETCLR | DMAF_BLITTER;
  BitmapClear(screen, DEPTH);

  cp = NewCopList(100);
  CopInit(cp);
  CopMakeDispWin(cp, X(0), Y(0), WIDTH, HEIGHT);
  CopMakePlayfield(cp, bplptr, screen, DEPTH);
  CopLoadPal(cp, palette, 0);
  CopEnd(cp);

  CopListActivate(cp);
  custom->dmacon = DMAF_SETCLR | DMAF_RASTER;
}

static void Kill() {
  custom->dmacon = DMAF_COPPER | DMAF_RASTER | DMAF_BLITTER;

  DeleteCopList(cp);
}

static void DrawSpans(UBYTE *bpl) {
  UBYTE *frame = anim->frame[anim->current];
  WORD f = normfx(SIN(frameCount * 32) * 48);
  WORD n = anim->height;
  WORD stride = screen->bytesPerRow;

  while (--n >= 0) {
    WORD m = *frame++;

    while (--m >= 0) {
      WORD x = *frame++;
      x += f;
      bset(bpl + (x >> 3), ~x);
    }

    bpl += stride;
  }

  anim->current++;

  if (anim->current >= anim->count)
    anim->current -= anim->count;
}

static void Render() {
  // LONG lines = ReadLineCounter();
  {
    BlitterClearSync(screen, active);
    DrawSpans(screen->planes[active]);
    BlitterFillSync(screen, active);
  }
  // Log("anim: %ld\n", ReadLineCounter() - lines);

  WaitVBlank();

  {
    WORD n = DEPTH;

    while (--n >= 0) {
      WORD i = (active + n + 1 - DEPTH) % (DEPTH + 1);
      if (i < 0)
        i += DEPTH + 1;
      CopInsSet32(bplptr[n], screen->planes[i]);
    }
  }

  active = (active + 1) % (DEPTH + 1);
}

EffectT Effect = { Load, UnLoad, Init, Kill, Render };
