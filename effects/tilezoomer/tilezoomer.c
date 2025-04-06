#include <effect.h>
#include <blitter.h>
#include <copper.h>
#include <stdlib.h>
#include <system/memory.h>

#define MOTIONBLUR 1

/* Add tile sized margins on every side to hide visual artifacts. */
#define MARGIN  32
#define WIDTH   (256 + MARGIN)
#define HEIGHT  (224 + MARGIN)

#if MOTIONBLUR
#define DEPTH   1
#define SHADOW  4
#else
#define DEPTH   2
#define SHADOW  0
#endif

#define TILESIZE  16
#define WTILES    (WIDTH / TILESIZE)
#define HTILES    (HEIGHT / TILESIZE)
#define NTILES    ((WTILES - 1) * (HTILES - 1))
#define ROTATION  1
#define ZOOM      1

static BitmapT *screen0;
#if !MOTIONBLUR
static BitmapT *screen1;
#endif
static int active = 0;
static CopListT *cp;
static CopInsPairT *bplptr;

/* for each tile following array stores source and destination offsets relative
 * to the beginning of a bitplane */
static int tiles[NTILES * 2];

#include "data/tilezoomer-pal.c"

static void CalculateTiles(int *tile, short rotation, short zoom) { 
  short x, y;
  short dx, dy;

  for (y = 0, dy = 0; y < HTILES - 1; y++, dy += TILESIZE) {
    short yo = (HTILES - 1 - y) - TILESIZE / 2;
    for (x = 0, dx = 0; x < WTILES - 1; x++, dx += TILESIZE) {
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

      *tile++ = sy * WIDTH * DEPTH + sx; /* source tile offset */
      *tile++ = dy * WIDTH * DEPTH + dx; /* destination tile offset */
    }
  }
}

static CopListT *MakeCopperList(void) {
  CopListT *cp = NewCopList(100);
#if MOTIONBLUR
  bplptr = CopSetupBitplanes(cp, screen0, SHADOW);
  CopMove16(cp, bpl1mod, MARGIN / 8);
  CopMove16(cp, bpl2mod, MARGIN / 8);
#else
  bplptr = CopSetupBitplanes(cp, screen0, DEPTH);
  /* Screen bitplanes are interleaved! */
  CopMove16(cp, bpl1mod, (WIDTH * (DEPTH - 1) + MARGIN) / 8);
  CopMove16(cp, bpl2mod, (WIDTH * (DEPTH - 1) + MARGIN) / 8);
#endif
  return CopListFinish(cp);
}

static void Init(void) {
  CalculateTiles(tiles, ROTATION, ZOOM);

#if MOTIONBLUR
  screen0 = NewBitmap(WIDTH, HEIGHT, DEPTH + SHADOW, BM_CLEAR);
#else
  screen0 = NewBitmap(WIDTH, HEIGHT, DEPTH, BM_CLEAR | BM_INTERLEAVED);
  screen1 = NewBitmap(WIDTH, HEIGHT, DEPTH, BM_CLEAR | BM_INTERLEAVED);
#endif

#if MOTIONBLUR
  SetupPlayfield(MODE_LORES, SHADOW, X(MARGIN), Y((256 - HEIGHT + MARGIN) / 2),
                 WIDTH - MARGIN, HEIGHT - MARGIN);
  LoadColors(tilezoomer_colors, 0);
#else
  SetupPlayfield(MODE_LORES, DEPTH, X(MARGIN), Y((256 - HEIGHT + MARGIN) / 2),
                 WIDTH - MARGIN, HEIGHT - MARGIN);
  SetColor(0, 0x000);
  SetColor(1, 0x44f);
  SetColor(2, 0x88f);
  SetColor(3, 0xccf);
#endif

  cp = MakeCopperList();
  CopListActivate(cp);
  EnableDMA(DMAF_RASTER | DMAF_BLITTER | DMAF_BLITHOG);
}

static void Kill(void) {
  DeleteBitmap(screen0);
#if !MOTIONBLUR
  DeleteBitmap(screen1);
#endif
  DeleteCopList(cp);
}

static void DrawSeed(u_char *bpl) {
  short y = HEIGHT / 2 - TILESIZE / 2;
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

static void MoveTiles(void *src, void *dst, short xshift, short yshift) {
  short n = NTILES - 1;
  int *tile = tiles;
  int offset;

  register volatile void *bltptr asm("a4") = &custom->bltcon1;

  custom->bltadat = 0xffff;
  custom->bltbmod = BLTMOD;
  custom->bltcmod = BLTMOD;
  custom->bltdmod = BLTMOD;
  custom->bltcon0 = (SRCB | SRCC | DEST) | (ABC | ABNC | NABC | NANBC);

  offset = yshift * WIDTH * DEPTH + xshift;

  do {
    int srcoff = *tile++ + offset;
    int dstoff = *tile++ + offset;
    register void *srcpt asm("a2") = src + ((srcoff >> 3) & ~1);
    register void *dstpt asm("a3") = dst + ((dstoff >> 3) & ~1);
    short sx = srcoff;
    short dx = dstoff;
    u_short bltcon1;
    u_int mask;
    short shift;

    sx &= 15; dx &= 15;

    shift = dx - sx;
    if (shift >= 0) {
      mask = 0xffff0000;
      bltcon1 = rorw(shift, 4);
    } else {
      mask = 0x0000ffff;
      bltcon1 = rorw(-shift, 4) | BLITREVERSE;

      srcpt += WIDTH * DEPTH * (TILESIZE - 1) / 8 + 2;
      dstpt += WIDTH * DEPTH * (TILESIZE - 1) / 8 + 2;
    }

    mask = rorl(mask, dx);

#if 1
    {
      volatile void *ptr = bltptr;

      WaitBlitter();

      stwi(ptr, bltcon1);
      stli(ptr, mask);
      stli(ptr, dstpt);
      stli(ptr, srcpt);
      ptr += 4;
      stli(ptr, dstpt);
      stwi(ptr, BLTSIZE);
    }
#else
    WaitBlitter();

    custom->bltcon1 = bltcon1;
    custom->bltalwm = mask;
    custom->bltafwm = swap16(mask);
    custom->bltcpt = dstpt;
    custom->bltbpt = srcpt;
    custom->bltdpt = dstpt;
    custom->bltsize = BLTSIZE;
#endif
  } while (--n >= 0);
}

static void UpdateBitplanePointers(void) {
  int offset = (TILESIZE + WIDTH * DEPTH * TILESIZE) / 8;
  short i;
#if MOTIONBLUR
  short j = active;
  for (i = SHADOW - 1; i >= 0; i--) {
    CopInsSet32(&bplptr[i], screen0->planes[j] + offset);
    j--;
    if (j < 0)
      j += DEPTH + SHADOW;
  }
#else
  for (i = 0; i < DEPTH; i++)
    CopInsSet32(&bplptr[i], screen1->planes[i] + offset);
#endif
}

PROFILE(TileZoomer);

static void Render(void) {
#if MOTIONBLUR
  static short rotation = 0;
  static short zoom = 0;
#endif
  ProfilerStart(TileZoomer);
#if MOTIONBLUR
  if ((frameCount & 63) < 2) {
    rotation = random() % 2;
    zoom = random() % 2;
    if (rotation == 0)
      rotation = -1;
    CalculateTiles(tiles, rotation, zoom);
  }
#endif
  if (zoom && ((random() & 3) == 0))
    DrawSeed(screen0->planes[active]);
  {
    short xshift = random() & (TILESIZE - 1);
    short yshift = random() & (TILESIZE - 1);
    void *src = screen0->planes[active];
    void *dst;
#if MOTIONBLUR
    if (++active == DEPTH + SHADOW)
      active = 0;
    dst = screen0->planes[active];
#else
    dst = screen1->planes[0];
#endif

    MoveTiles(src, dst, xshift, yshift);
  }
  UpdateBitplanePointers();
  ProfilerStop(TileZoomer);

  TaskWaitVBlank();
#if !MOTIONBLUR
  swapr(screen0, screen1);
#endif
}

EFFECT(TileZoomer, NULL, NULL, Init, Kill, Render, NULL);
