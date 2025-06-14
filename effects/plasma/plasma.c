#include <effect.h>
#include <blitter.h>
#include <copper.h>
#include <fx.h>
#include <pixmap.h>
#include <system/memory.h>

#define WIDTH (320 - 32)
#define HEIGHT 256
#define DEPTH 0

#define HTILES (WIDTH / 8)
#define VTILES (HEIGHT / 4)

/* Don't change these settings without reading a note about copper chunky! */
#undef DIWVP
#define DIWVP 0x2c
#undef DIWHP
#define DIWHP 0x88

static CopInsT *chunky[2][VTILES];
static CopListT *cp[2];
static int active = 0;

#include "data/plasma-colors.c"

static char tab1[256], tab2[256], tab3[256];
static u_char a0, a1, a2, a3, a4;
static u_char xbuf[HTILES], ybuf[VTILES];

static void GeneratePlasmaTables(void) {
  short i;

  /* Generate plasma tables */
  for (i = 0; i < 256; i++) {
    short rad = i * 16;

    tab1[i] = fx4i(3 * 47) * SIN(rad * 2) >> 16;
    tab2[i] = fx4i(3 * 31) * COS(rad * 2) >> 16;
    tab3[i] = fx4i(3 * 37) * SIN(rad * 2) >> 16;
  }
}

static void Load(void) {
  GeneratePlasmaTables();
}

/* Double-buffered copper chunky with 8x4 pixels.
 *
 * In order to make copper run same stream of instructions (i.e. color line)
 * 4 times, the screen has to be constructed in a very specific way.
 *
 * Firstly, vertical start position must be divisible by 4. This is driven by
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
static CopListT *MakeCopperList(CopInsT **row) {
  CopListT *cp = NewCopList(80 + (HTILES + 5) * VTILES);
  short x, y;

  CopWait(cp, Y(0), HP(0));

  for (y = 0; y < VTILES; y++) {
    CopInsPairT *location = CopMove32(cp, cop2lc, 0);
    CopInsT *label = CopWaitH(cp, Y(y * 4), X(-4));
    CopInsSet32(location, label);
    row[y] = CopSetColor(cp, 0, 0);
    for (x = 0; x < HTILES - 1; x++)
      CopSetColor(cp, 0, 0); /* Last CopIns finishes at HP=0xD2 */
    CopSetColor(cp, 0, 0); /* set background to black, finishes at HP=0xD6 */
    CopSkip(cp, Y(y * 4 + 3), LASTHP); /* finishes at HP=0xDE */
    CopMove16(cp, copjmp2, 0);
  }

  return CopListFinish(cp);
}

static void Init(void) {
  cp[0] = MakeCopperList(chunky[0]);
  cp[1] = MakeCopperList(chunky[1]);

  CopListActivate(cp[1]);
}

static void Kill(void) {
  DisableDMA(DMAF_COPPER);

  DeleteCopList(cp[0]);
  DeleteCopList(cp[1]);
}

static void UpdateXBUF(void) {
  u_char _a0 = a0;
  u_char _a1 = a1;
  u_char _a2 = a2;
  u_char *_buf = xbuf;
  short n = HTILES - 1;

  do {
    *_buf++ = tab1[_a0] + tab2[_a1] + tab3[_a2];
    _a0 += 1; _a1 += 2; _a2 += 3;
  } while (--n >= 0);
}

static void UpdateYBUF(void) {
  u_char _a3 = a3;
  u_char _a4 = a4;
  u_char *_buf = ybuf;
  short n = VTILES - 1;

  do {
    *_buf++ = tab1[_a3] + tab2[_a4];
    _a3 += 2; _a4 += 3;
  } while (--n >= 0);
}

#define OPTIMIZED 1

static void UpdateChunky(void) {
  u_short *cmap = colors.pixels;
  short y;

  UpdateXBUF();
  UpdateYBUF();

  a0 += 1; a1 += 3; a2 += 2; a3 += 1; a4 -= 1;

  for (y = 0; y < VTILES; y++) {
    short *ins = &chunky[active][y]->move.data;

    short x = HTILES - 1;
    do {
#if OPTIMIZED
      asm volatile("moveq #0,d0\n"
                   "moveb %1@(%2:w),d0\n"
                   "addb  %3@(%4:w),d0\n"
                   "addw  d0,d0\n"
                   "movew %5@(d0:w),%0@+\n"
                   "addql #2,%0\n"
                   : "+a" (ins)
                   : "a" (xbuf), "d" (x), "a" (ybuf), "d" (y), "a" (cmap)
                   : "d0");
#else
      u_char v = xbuf[x] + ybuf[y];
      *ins++ = cmap[v];
      ins++;
#endif
    } while (--x >= 0);
  }
}

PROFILE(RenderPlasma);

static void Render(void) {
  ProfilerStart(RenderPlasma);
  {
    UpdateChunky();
  }
  ProfilerStop(RenderPlasma);

  CopListRun(cp[active]);
  TaskWaitVBlank();
  active ^= 1;
}

EFFECT(Plasma, Load, NULL, Init, Kill, Render, NULL);
