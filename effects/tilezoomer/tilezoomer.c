#include <effect.h>
#include <blitter.h>
#include <copper.h>
#include <stdlib.h>
#include <system/memory.h>

/* Add tile sized margins on every side to hide visual artifacts. */
#define MARGIN  32
#define WIDTH   (256 + MARGIN)
#define HEIGHT  (256 + MARGIN)
#define DEPTH   2

#define TILESIZE  16
#define TILES     (WIDTH / TILESIZE)
#define ROTATION  1
#define ZOOM      1

static BitmapT *screen0, *screen1;
static CopListT *cp;
static CopInsT *bplptr[DEPTH];
static int tiles[(TILES - 1) * (TILES - 1) * 4];

static void CalculateTiles(int *tile, short rotation, short zoom) { 
  short x, y;
  short dx, dy;

  for (y = 0, dy = 0; y < TILES - 1; y++, dy += TILESIZE) {
    short yo = (TILES - y) - TILESIZE / 2;
    for (x = 0, dx = 0; x < TILES - 1; x++, dx += TILESIZE) {
      short xo = x - TILESIZE / 2;
      short sx = dx;
      short sy = dy;

      if (rotation > 0) {
        sx += yo;
        sy += xo;
      } else if (rotation < 0) {
        sx -= yo;
        sy -= xo;
      }

      if (zoom > 0) {
        sx -= xo;
        sy += yo;
      } else if (zoom < 0) {
        sx += xo;
        sy -= yo;
      }

      *tile++ = sy * WIDTH * DEPTH + sx;
      *tile++ = dy * WIDTH * DEPTH + dx;
    }
  }
}

static void Init(void) {
  CalculateTiles(tiles, ROTATION, ZOOM);

  screen0 = NewBitmapCustom(WIDTH, HEIGHT, DEPTH,
                            BM_CLEAR|BM_DISPLAYABLE|BM_INTERLEAVED);
  screen1 = NewBitmapCustom(WIDTH, HEIGHT, DEPTH,
                            BM_CLEAR|BM_DISPLAYABLE|BM_INTERLEAVED);

  SetupPlayfield(MODE_LORES, DEPTH, X(32), Y(0), 256, 256);
  SetColor(0, 0x000);
  SetColor(1, 0x44f);
  SetColor(2, 0x88f);
  SetColor(3, 0xccf);

  cp = NewCopList(100);
  CopInit(cp);
  CopSetupBitplanes(cp, bplptr, screen0, DEPTH);
  /* Screen bitplanes are interleaved! */
  CopMove16(cp, bpl1mod, (WIDTH * (DEPTH - 1) + MARGIN) / 8);
  CopMove16(cp, bpl2mod, (WIDTH * (DEPTH - 1) + MARGIN) / 8);
  CopEnd(cp);

  CopListActivate(cp);
  EnableDMA(DMAF_RASTER | DMAF_BLITTER | DMAF_BLITHOG);
}

static void Kill(void) {
  DeleteBitmap(screen0);
  DeleteBitmap(screen1);
  DeleteCopList(cp);
}

static void DrawSeed(void) {
  u_char *bpl = screen0->planes[0];
  short y = HEIGHT / 2 + 2 * TILESIZE - TILESIZE / 4;
  short x = WIDTH / 2 - TILESIZE / 2;
  int offset = (y * WIDTH * DEPTH + x) / 8;
  short n = DEPTH * 8 - 1;

  do {
    bpl[offset] = random();
    offset += WIDTH / 8;
  } while (--n >= 0);
}

#define BLTMOD (WIDTH / 8 - TILESIZE / 8 - 2)
#define BLTSIZE ((TILESIZE * DEPTH << 6) | ((TILESIZE + 16) >> 4))

void MoveTiles(void) {
  void *src = screen0->planes[0];
  void *dst = screen1->planes[0];
  short xshift = random() & (TILESIZE - 1);
  short yshift = random() & (TILESIZE - 1);
  short n = (TILES - 1) * (TILES - 1) - 1;
  int *tile = tiles;
  int offset;

  custom->bltadat = 0xffff;
  custom->bltbmod = BLTMOD;
  custom->bltcmod = BLTMOD;
  custom->bltdmod = BLTMOD;
  custom->bltcon0 = (SRCB | SRCC | DEST) | (ABC | ABNC | NABC | NANBC);

  offset = yshift * WIDTH * DEPTH + xshift;

  do {
    int srcoff = *tile++ + offset;
    int dstoff = *tile++ + offset;
    void *srcpt = src + ((srcoff >> 3) & ~1);
    void *dstpt = dst + ((dstoff >> 3) & ~1);
    short sx = srcoff, dx = dstoff;
    u_short bltcon1;
    u_int mask;

    sx &= 15; dx &= 15;

    if (dx >= sx) {
      mask = 0xffff0000;
      bltcon1 = rorw((dx - sx) & 15, 4);
    } else {
      mask = 0x0000ffff;
      bltcon1 = rorw((sx - dx) & 15, 4) | BLITREVERSE;

      srcpt += WIDTH * DEPTH * (TILESIZE - 1) / 8 + 2;
      dstpt += WIDTH * DEPTH * (TILESIZE - 1) / 8 + 2;
    }

    mask = rorl(mask, dx);

    WaitBlitter();

    custom->bltcon1 = bltcon1;
    custom->bltalwm = mask;
    custom->bltafwm = swap16(mask);
    custom->bltcpt = dstpt;
    custom->bltbpt = srcpt;
    custom->bltdpt = dstpt;
    custom->bltsize = BLTSIZE;
  } while (--n >= 0);
}

PROFILE(TileZoomer);

static void Render(void) {
  ProfilerStart(TileZoomer);
  {
    DrawSeed();
    MoveTiles();
  }
  ProfilerStop(TileZoomer);

  {
    int offset = (TILESIZE + WIDTH * DEPTH * TILESIZE) / 8;
    short i;
    for (i = 0; i < DEPTH; i++)
      CopInsSet32(bplptr[i], screen1->planes[i] + offset);
  }
  TaskWaitVBlank();
  swapr(screen0, screen1);
}

EFFECT(tilezoomer, NULL, NULL, Init, Kill, Render);
