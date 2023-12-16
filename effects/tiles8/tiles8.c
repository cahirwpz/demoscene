#include <effect.h>
#include <blitter.h>
#include <copper.h>
#include <fx.h>
#include <pixmap.h>
#include <stdlib.h>
#include <system/memory.h>

#define WIDTH (320 - 16)
#define HEIGHT 256
#define DEPTH 1

#define HTILES (WIDTH / 8)
#define VTILES (HEIGHT / 8)

#undef X
#undef Y

/* Don't change these settings without reading a note about copper chunky! */
#define Y(y) ((y) + 0x28)
#define VP(y) (Y(y) & 255)
#define X(x) ((x) + 0x84)
#define HP(x) (X(x) / 2)

static CopInsPairT *bplptr;
static CopInsT *chunky[VTILES];
static BitmapT *screen[2];
static CopListT *cp;
static u_short *tilescr;
static short ntiles;
static short active;

#include "data/twist.c"
#include "data/twist-colors.c"
#include "data/tiles-c.c"

static void Load(void) {
  short x, y, i;

  ntiles = tilegfx.height / 8;
  tilescr = MemAlloc(HTILES * VTILES * sizeof(u_short), MEMF_PUBLIC);

  {
    // u_char *pixels = (u_char *)twist.pixels;

    for (i = 0, y = 0; y < VTILES; y++)
      for (x = 0; x < HTILES; x++, i++) {
        u_short v = random();
        // u_short v = pixels[i] >> 4;
        tilescr[i] = (v & (ntiles - 1)) << 4;
      }
  }
}

static CopListT *MakeCopperList(void) {
  CopListT *cp = NewCopList(80 + (HTILES + 4) * VTILES);

  /* X(-1) to align with copper induced color changes */
  bplptr = CopSetupBitplanes(cp, screen[1], DEPTH);
  CopWaitV(cp, VP(0));

  /* Copper Chunky.
   *
   * In order to make copper run same stream of instructions (i.e. color line)
   * 8 times, the screen has to be constructed in a very specific way.
   *
   * Firstly, vertical start position must be divisible by 8. This is driven by
   * copper lack of capability to mask out the most significant bit in WAIT and
   * SKIP instruction. This causes a glitch after transition from line 127 to
   * line 128. The problem is mentioned in HRM:
   * http://amigadev.elowar.com/read/ADCD_2.1/Hardware_Manual_guide/node005B.html
   *
   * Secondly, a problem arises when transitioning from line 255 to line 256.
   * Inserted SKIP instruction effectively awaits beam position that won't ever
   * happen! To avoid it each SKIP instruction must be guaranteed to eventually
   * trigger. Thus they must be placed just before the end of scan line.
   * UAE copper debugger facility was used to find the right spot.
   */
  {
    short x, y;
    for (y = 0; y < VTILES; y++) {
      CopInsPairT *location = CopMove32(cp, cop2lc, 0);
      CopInsT *label = CopWaitH(cp, VP(y * 8), HP(-4));
      CopInsSet32(location, label);
      chunky[y] = CopSetColor(cp, 1, 0);
      for (x = 0; x < HTILES - 1; x++)
        CopSetColor(cp, 1, 0); /* Last CopIns finishes at HP=0xD6 */
      CopSkip(cp, VP(y * 8 + 7), 0xDE); /* finishes at HP=0xDE */
      CopMove16(cp, copjmp2, 0);
    }
  }
  return CopListFinish(cp);
}

static void Init(void) {
  screen[0] = NewBitmap(WIDTH, HEIGHT, DEPTH, BM_CLEAR);
  screen[1] = NewBitmap(WIDTH, HEIGHT, DEPTH, BM_CLEAR);

  SetupPlayfield(MODE_LORES, DEPTH, X(0), Y(0), WIDTH, HEIGHT);
  LoadColors(tilegfx_colors, 0);

  cp = MakeCopperList();
  CopListActivate(cp);

  EnableDMA(DMAF_RASTER | DMAF_BLITTER | DMAF_BLITHOG);
}

static void Kill(void) {
  DisableDMA(DMAF_COPPER | DMAF_RASTER | DMAF_BLITTER | DMAF_BLITHOG);

  DeleteCopList(cp);
}

static void UpdateChunky(void) {
  u_char *data = twist.pixels;
  u_short *cmap = colors.pixels;
  u_char offset = SIN(frameCount * 8) >> 2;
  short y;

  for (y = 0; y < VTILES; y++) {
    short *row = &chunky[y]->move.data;

    short x = HTILES - 1;
    do {
#if 0
      u_char p = *data++ - offset;
      *row++ = getword(cmap, p);
#else
      asm volatile("moveq #0,d0\n"
                   "moveb %0@+,d0\n"
                   "addb %3,d0\n"
                   "addw d0,d0\n"
                   "movew %2@(d0:w),%1@+\n"
                   : "+a" (data), "+a" (row)
                   : "a" (cmap), "d" (offset)
                   : "d0");
#endif
      row++;
    } while (--x >= 0);
  }
}

static void UpdateTiles(void) {
  u_int *_tilescr = (u_int *)tilescr;
  register u_int incr asm("d2") = 0x00100010;
  register u_int mask asm("d3") = 0x00f000f0;
  short i;

  for (i = 0; i < VTILES * HTILES / 4; i++) {
    *_tilescr = (*_tilescr + incr) & mask; _tilescr++;
    *_tilescr = (*_tilescr + incr) & mask; _tilescr++;
  }
}

static void RenderTiles(void) {
  u_short *_tilescr = tilescr;
  u_short *dst = screen[active]->planes[0];
  u_short bltsize = (8 << 6) + 1;
  void *_tile = tilegfx.planes[0];
  void *custom_ = (void *)&custom->bltbpt;

  custom->bltamod = 0;
  custom->bltbmod = 0;
  custom->bltcon0 = (SRCA | SRCB | DEST) | (ABC | ANBC | ABNC | NABNC);
  custom->bltcon1 = 0;
  custom->bltafwm = -1;
  custom->bltalwm = -1;
  custom->bltcdat = 0xff00;
  custom->bltdmod = WIDTH / 8 - 2;


  {
    short y = VTILES - 1;
    do {
      short x = HTILES / 2 - 1;

      do {
#if 0
        // WaitBlitter();
        custom->bltbpt = _tile + *tilenums++;
        custom->bltapt = _tile + *tilenums++;
        custom->bltdpt = dst;
        custom->bltsize = bltsize;
#else
        asm volatile("movel %0,a0\n"
                     "movel %3,a1\n"
                     "addaw %4@+,a1\n"
                     "movel %3,a2\n"
                     "addaw %4@+,a2\n"
                     "movel a1,a0@+\n"
                     "movel a2,a0@+\n"
                     "movel %1,a0@+\n"
                     "movew %2,a0@\n"
                     : 
                     : "a" (custom_), "a" (dst), "d" (bltsize), "d" (_tile), "a" (_tilescr)
                     : "a0", "a1", "a2");
#endif
        dst++;
      } while (--x >= 0);

      dst += (WIDTH - WIDTH / 8) / 2;
    } while (--y >= 0);
  }
}

PROFILE(UpdateTiles);
PROFILE(RenderTiles);

static void Render(void) {
  ProfilerStart(UpdateTiles);
  {
    UpdateChunky();
    UpdateTiles();
  }
  ProfilerStop(UpdateTiles);
  
  ProfilerStart(RenderTiles);
  {
    RenderTiles();
  }
  ProfilerStop(RenderTiles);

  CopUpdateBitplanes(bplptr, screen[active], DEPTH);
  TaskWaitVBlank();
  active ^= 1;
}

EFFECT(Tiles8, Load, NULL, Init, Kill, Render, NULL);
