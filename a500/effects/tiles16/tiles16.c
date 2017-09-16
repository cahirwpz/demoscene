#include "startup.h"
#include "hardware.h"
#include "coplist.h"
#include "memory.h"
#include "gfx.h"
#include "ilbm.h"
#include "blitter.h"
#include "random.h"
#include "tasks.h"

STRPTR __cwdpath = "data";

#define WIDTH 320
#define HEIGHT 256
#define DEPTH 4

#define VTILES (HEIGHT / 16)
#define HTILES (WIDTH / 16)
#define TILESIZE (16 * 16 * DEPTH / 8)

static CopInsT *bplptr[DEPTH];
static BitmapT *screen0, *screen1;
static BitmapT *tiles;
static CopListT *cp;
static UWORD *tilescr;

static void Load() {
  tiles = LoadILBMCustom("tiles.ilbm",
                         BM_DISPLAYABLE | BM_INTERLEAVED | BM_LOAD_PALETTE);
  tilescr = MemAlloc(sizeof(UWORD) * VTILES * HTILES, MEMF_PUBLIC);

  {
    WORD i;

    for (i = 0; i < VTILES * HTILES; i++)
      tilescr[i] = (random() & 63) * TILESIZE;
  }
}

static void UnLoad() {
  DeletePalette(tiles->palette);
  DeleteBitmap(tiles);
}

static void Init() {
  /* Use interleaved mode to limit number of issued blitter operations. */
  screen0 = NewBitmapCustom(WIDTH, HEIGHT, DEPTH,
                            BM_CLEAR | BM_DISPLAYABLE | BM_INTERLEAVED);
  screen1 = NewBitmapCustom(WIDTH, HEIGHT, DEPTH,
                            BM_CLEAR | BM_DISPLAYABLE | BM_INTERLEAVED);

  cp = NewCopList(100);
  CopInit(cp);
  CopSetupGfxSimple(cp, MODE_LORES, DEPTH, X(0), Y(0), WIDTH, HEIGHT);
  CopSetupBitplanes(cp, bplptr, screen0, DEPTH);
  CopLoadPal(cp, tiles->palette, 0);
  CopEnd(cp);

  CopListActivate(cp);

  EnableDMA(DMAF_RASTER | DMAF_BLITTER | DMAF_BLITHOG);
}

static void Kill() {
  DisableDMA(DMAF_COPPER | DMAF_RASTER | DMAF_BLITTER | DMAF_BLITHOG);

  DeleteCopList(cp);

  DeleteBitmap(screen0);
  DeleteBitmap(screen1);
}

#define OPTIMIZED 1

static void UpdateTiles() {
  UWORD *_tile = tilescr;
  APTR src = tiles->planes[0];
  APTR dst = screen0->planes[0];
  WORD size = ((16 * DEPTH) << 6) | 1;
#if OPTIMIZED
  LONG bit = 6;
  APTR _custom = (APTR)&custom->bltapt;
#endif

  WaitBlitter();

  custom->bltafwm = -1;
  custom->bltalwm = -1;
  custom->bltamod = 0;
  custom->bltdmod = (WIDTH - 16) / 8;
  custom->bltcon0 = (SRCA | DEST | A_TO_D);
  custom->bltcon1 = 0;

  {
    WORD y = VTILES - 1;
    do {
      WORD x = HTILES - 1;

      do {
#if OPTIMIZED
        asm volatile("movel %0,a0\n"
                     "movel %1,a1\n"
                     "addw  %5@+,a1\n"
                     "1: btst %4,a0@(-78)\n"
                     "bnes  1b\n"
                     "movel a1,a0@+\n"
                     "movel %2,a0@+\n"
                     "movew %3,a0@\n"
                     :
                     : "d" (_custom), "d" (src), "d" (dst), "d" (size), "d" (bit), "a" (_tile)
                     : "a0", "a1");
#else
        APTR _src = src + *_tile++;
        WaitBlitter();
        custom->bltapt = _src;
        custom->bltdpt = dst;
        custom->bltsize = size;
#endif

        dst += 2;
      } while (--x >= 0);

      dst += WIDTH * (DEPTH - 1) / 8;
      dst += 15 * WIDTH * DEPTH / 8;
    } while (--y >= 0);
  }
}

static void Render() {
  LONG lines = ReadLineCounter();
  UpdateTiles();
  Log("update: %ld\n", ReadLineCounter() - lines);

  CopUpdateBitplanes(bplptr, screen0, DEPTH);
  TaskWait(VBlankEvent);
  swapr(screen0, screen1);
}

EffectT Effect = { Load, UnLoad, Init, Kill, Render };
