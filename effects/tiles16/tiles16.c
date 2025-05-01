#include <effect.h>
#include <blitter.h>
#include <copper.h>
#include <gfx.h>
#include <system/memory.h>

#define TILEW 16
#define TILEH 16

#define WIDTH (320 + TILEW)
#define HEIGHT (256 + TILEH)
#define DEPTH 5
#define BPLMOD (WIDTH * (DEPTH - 1) / 8)

#define VTILES (HEIGHT / TILEH)
#define HTILES (WIDTH / TILEW)
#define TILESIZE (TILEW * TILEH * DEPTH / 8)

static CopInsPairT *bplptr[2];
static BitmapT *screen[2];
static CopInsT *bplcon1[2];
static CopListT *cp[2];
static void **tileptrs;
static short active;

#include "data/MagicLand-map.c"
#include "data/MagicLand-tiles.c"
#define tilecount MagicLand_ntiles
#define tilemap_width MagicLand_map_width
#define tilemap_height MagicLand_map_height
#define tilemap ((short *)MagicLand_map)

static void Load(void) {
  tileptrs = MemAlloc(sizeof(void *) * tilecount, MEMF_PUBLIC);
  {
    short n = tilecount;
    void *base = tiles.planes[0];
    void **ptrs = tileptrs;
    while (--n >= 0) {
      *ptrs++ = base;
      base += TILESIZE;
    }
  }

  {
    int i;
    for (i = 0; i < tilemap_width * tilemap_height; i++) {
      tilemap[i] <<= 2;
      tilemap[i] |= 3;
    }
  }
}

static CopListT *MakeCopperList(int i) {
  CopListT *cp = NewCopList(100);
  bplptr[i] = CopSetupBitplanes(cp, screen[i], DEPTH);
  bplcon1[i] = CopMove16(cp, bplcon1, 0);
  return CopListFinish(cp);
}

static void Init(void) {
  /* extra memory for horizontal scrolling */
  short extra = tilemap_width * TILEW / WIDTH;

  Log("Allocate %d extra lines!\n", extra);

  /* Use interleaved mode to limit number of issued blitter operations. */
  screen[0] = NewBitmap(WIDTH, HEIGHT + extra, DEPTH,
                        BM_CLEAR | BM_INTERLEAVED);
  screen[1] = NewBitmap(WIDTH, HEIGHT + extra, DEPTH,
                        BM_CLEAR | BM_INTERLEAVED);

  SetupMode(MODE_LORES, DEPTH);
  SetupDisplayWindow(MODE_LORES, X(0), Y(0), WIDTH - 16, HEIGHT - 16);
  SetupBitplaneFetch(MODE_LORES, X(-16), WIDTH);
  LoadColors(tiles_colors, 0);

  cp[0] = MakeCopperList(0);
  cp[1] = MakeCopperList(1);

  CopListActivate(cp[1]);

  EnableDMA(DMAF_RASTER | DMAF_BLITTER | DMAF_BLITHOG);
}

static void Kill(void) {
  DisableDMA(DMAF_COPPER | DMAF_RASTER | DMAF_BLITTER | DMAF_BLITHOG);

  DeleteCopList(cp[0]);
  DeleteCopList(cp[1]);

  DeleteBitmap(screen[0]);
  DeleteBitmap(screen[1]);
}

void TriggerRefresh(short x, short y, short w __unused, short h __unused)
{
  short *map = tilemap;
  int tilemod = tilemap_width - HTILES;

  map += x + (short)y * (short)tilemap_width;

  {
    short j = VTILES - 1;
    do {
      short i = HTILES - 1;

      do {
        *map++ |= 3;
      } while (--i >= 0);

      map += tilemod;
    } while (--j >= 0);
  }
}

#define WAITBLT()                               \
  asm("1: btst #6,%0@(2)\n" /* dmaconr */       \
      "   bnes 1b"                              \
      :: "a" (custom));

static void UpdateTiles(BitmapT *screen, short x, short y,
                        CustomPtrT custom_ asm("a6"))
{
  short *map = tilemap;
  void *ptrs = tileptrs;
  void *dst = screen->planes[0] + (x << 1);
  short size = ((TILEH * DEPTH) << 6) | 1;
  int tilemod = tilemap_width - HTILES;
  short current = active + 1;

  map += x + (short)y * (short)tilemap_width;

  WAITBLT();

  custom_->bltafwm = -1;
  custom_->bltalwm = -1;
  custom_->bltamod = 0;
  custom_->bltdmod = (WIDTH - TILEW) / 8;
  custom_->bltcon0 = (SRCA | DEST | A_TO_D);
  custom_->bltcon1 = 0;

  {
    short j = VTILES - 1;

    do {
      short i = HTILES - 1;

      do {
        short tile = *map++;

        if (tile & current) {
          void *src = *(void **)(ptrs + (tile & ~3));

          WAITBLT();
          custom_->bltapt = src;
          custom_->bltdpt = dst;
          custom_->bltsize = size;

          map[-1] ^= current;
        }

        dst += 2;
      } while (--i >= 0);

      map += tilemod;
      dst += BPLMOD + 15 * WIDTH * DEPTH / 8;
    } while (--j >= 0);
  }
}

PROFILE(Tiles16);

static void Render(void) {
  ProfilerStart(Tiles16);
  {
    short t = frameCount;
    short tile = t >> 4;
    short pixel = 15 - (t & 15);

    short x = tile % (tilemap_width - HTILES);
    short y = 35;

    UpdateTiles(screen[active], x, y, custom);

    {
      short i;
      CopInsPairT *_bplptr = bplptr[active];
      void **_planes = screen[active]->planes;
      int offset = x << 1;

      for (i = 0; i < DEPTH; i++)
        CopInsSet32(&_bplptr[i], _planes[i] + offset);
    }
    CopInsSet16(bplcon1[active], pixel | (pixel << 4));
    CopListRun(cp[active]);
  }
  ProfilerStop(Tiles16);

  TaskWaitVBlank();
  active ^= 1;
}

EFFECT(Tiles16, Load, NULL, Init, Kill, Render, NULL);
