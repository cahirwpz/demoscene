#include "startup.h"
#include "hardware.h"
#include "coplist.h"
#include "memory.h"
#include "gfx.h"
#include "ilbm.h"
#include "io.h"
#include "blitter.h"
#include "random.h"
#include "tasks.h"

STRPTR __cwdpath = "data";
LONG __chipmem = 300 * 1024;

#define WIDTH (320 + 16)
#define HEIGHT (256 + 16)
#define DEPTH 5
#define BPLMOD (WIDTH * (DEPTH - 1) / 8)

#define VTILES (HEIGHT / 16)
#define HTILES (WIDTH / 16)
#define TILESIZE (16 * 16 * DEPTH / 8)

static CopInsT *bplptr[2][DEPTH];
static BitmapT *screen[2];
static CopInsT *bplcon1[2];
static CopListT *cp[2];
static WORD active;

typedef struct TileSet {
  UWORD width, height;
  UWORD count;
  char *path;
  BitmapT *tiles;
  APTR *ptrs;
} TileSetT;

typedef struct TileMap {
  UWORD width, height;
  char *path;
  WORD *map;
} TileMapT;

#include "data/MagicLand.h"
#define Tiles MagicLand_tiles
#define Map MagicLand_map

static void Load() {
  Tiles.tiles = LoadILBMCustom(Tiles.path,
                               BM_DISPLAYABLE | BM_INTERLEAVED | BM_LOAD_PALETTE);
  Tiles.ptrs = MemAlloc(sizeof(APTR) * Tiles.count, MEMF_PUBLIC);
  {
    WORD n = Tiles.count;
    APTR base = Tiles.tiles->planes[0];
    APTR *ptrs = Tiles.ptrs;
    while (--n >= 0) {
      *ptrs++ = base;
      base += TILESIZE;
    }
  }

  Map.map = LoadFile(Map.path, MEMF_PUBLIC);
  {
    LONG n = Map.width * Map.height;
    LONG i;
    for (i = 0; i < n; i++) {
      Map.map[i] <<= 2;
      Map.map[i] |= 3;
    }
  }
}

static void UnLoad() {
  DeletePalette(Tiles.tiles->palette);
  DeleteBitmap(Tiles.tiles);
  MemFree(Map.map);
}

static void MakeCopperList(CopListT *cp, LONG i) {
  CopInit(cp);
  CopSetupMode(cp, MODE_LORES, DEPTH);
  CopSetupDisplayWindow(cp, MODE_LORES, X(0), Y(0), WIDTH - 16, HEIGHT - 16);
  CopSetupBitplaneFetch(cp, MODE_LORES, X(-16), WIDTH);
  CopSetupBitplanes(cp, bplptr[i], screen[i], DEPTH);
  bplcon1[i] = CopMove16(cp, bplcon1, 0);
  CopLoadPal(cp, Tiles.tiles->palette, 0);
  CopEnd(cp);
}

static void Init() {
  /* extra memory for horizontal scrolling */
  WORD extra = div16(Map.width * Tiles.width, WIDTH);

  Log("Allocate %ld extra lines!\n", (LONG)extra);

  /* Use interleaved mode to limit number of issued blitter operations. */
  screen[0] = NewBitmapCustom(WIDTH, HEIGHT + extra, DEPTH,
                              BM_CLEAR | BM_DISPLAYABLE | BM_INTERLEAVED);
  screen[1] = NewBitmapCustom(WIDTH, HEIGHT + extra, DEPTH,
                              BM_CLEAR | BM_DISPLAYABLE | BM_INTERLEAVED);

  cp[0] = NewCopList(100);
  MakeCopperList(cp[0], 0);
  cp[1] = NewCopList(100);
  MakeCopperList(cp[1], 1);

  CopListActivate(cp[1]);

  EnableDMA(DMAF_RASTER | DMAF_BLITTER | DMAF_BLITHOG);
}

static void Kill() {
  DisableDMA(DMAF_COPPER | DMAF_RASTER | DMAF_BLITTER | DMAF_BLITHOG);

  DeleteCopList(cp[0]);
  DeleteCopList(cp[1]);

  DeleteBitmap(screen[0]);
  DeleteBitmap(screen[1]);
}

__regargs void TriggerRefresh(WORD x, WORD y, WORD w, WORD h) {
  WORD *tiles = Map.map;
  LONG tilemod = Map.width - HTILES;

  tiles += x + (WORD)y * (WORD)Map.width;

  {
    WORD j = VTILES - 1;
    do {
      WORD i = HTILES - 1;

      do {
        *tiles++ |= 3;
      } while (--i >= 0);

      tiles += tilemod;
    } while (--j >= 0);
  }
}

#define WAITBLT()                               \
  asm("1: btst #6,%0@(2)\n" /* dmaconr */       \
      "   bnes 1b"                              \
      :: "a" (custom));

static __regargs void UpdateTiles(BitmapT *screen, WORD x, WORD y,
                                  volatile struct Custom* const custom asm("a6"))
{
  WORD *tiles = Map.map;
  APTR ptrs = Tiles.ptrs;
  APTR dst = screen->planes[0] + (x << 1);
  WORD size = ((16 * DEPTH) << 6) | 1;
  LONG tilemod = Map.width - HTILES;
  WORD current = active + 1;

  tiles += x + (WORD)y * (WORD)Map.width;

  WAITBLT();

  custom->bltafwm = -1;
  custom->bltalwm = -1;
  custom->bltamod = 0;
  custom->bltdmod = (WIDTH - 16) / 8;
  custom->bltcon0 = (SRCA | DEST | A_TO_D);
  custom->bltcon1 = 0;

  {
    WORD j = VTILES - 1;

    do {
      WORD i = HTILES - 1;

      do {
        WORD tile = *tiles++;

        if (tile & current) {
          APTR src = *(APTR *)(ptrs + (tile & ~3));

          WAITBLT();
          custom->bltapt = src;
          custom->bltdpt = dst;
          custom->bltsize = size;

          tiles[-1] ^= current;
        }

        dst += 2;
      } while (--i >= 0);

      tiles += tilemod;
      dst += BPLMOD + 15 * WIDTH * DEPTH / 8;
    } while (--j >= 0);
  }
}

static void Render() {
  LONG lines = ReadLineCounter();
  WORD t = frameCount;
  WORD tile = t >> 4;
  WORD pixel = 15 - (t & 15);

  WORD x = tile % (Map.width - HTILES);
  WORD y = 35;

  UpdateTiles(screen[active], x, y, custom);

  {
    WORD i;
    CopInsT **_bplptr = bplptr[active];
    APTR *_planes = screen[active]->planes;
    LONG offset = x << 1;

    for (i = 0; i < DEPTH; i++)
      CopInsSet32(_bplptr[i], _planes[i] + offset);
  }
  CopInsSet16(bplcon1[active], pixel | (pixel << 4));
  CopListRun(cp[active]);
  Log("all: %ld\n", ReadLineCounter() - lines);

  TaskWait(VBlankEvent);
  active ^= 1;
}

EffectT Effect = { Load, UnLoad, Init, Kill, Render };
