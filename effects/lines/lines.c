#include "effect.h"
#include "blitter.h"
#include "copper.h"
#include "line.h"

#define WIDTH 320
#define HEIGHT 256
#define DEPTH 1

/*
 * 0 -> BlitterLine (1706)
 * 1 -> CpuLine (11245)
 * 2 -> CpuEdge (8550)
 * 3 -> CpuLineOpt (10872)
 * 4 -> CpuEdgeOpt (8253)
 */
#define LINE 0

void CpuEdgeOpt(void *bpl asm("a0"), short stride asm("a1"),
                short xs asm("d0"), short ys asm("d1"),
                short xe asm("d2"), short ye asm("d3"));


void CpuLineOpt(void *bpl asm("a0"), short stride asm("a1"),
                short xs asm("d0"), short ys asm("d1"),
                short xe asm("d2"), short ye asm("d3"));

static BitmapT *screen;
static CopListT *cp;

static void Load(void) {
  screen = NewBitmap(WIDTH, HEIGHT, DEPTH);

  SetupPlayfield(MODE_LORES, DEPTH, X(0), Y(0), WIDTH, HEIGHT);
  SetColor(0, 0x000);
  SetColor(1, 0xfff);

  cp = NewCopList(100);
  CopInit(cp);
  CopSetupBitplanes(cp, screen, DEPTH);
  CopEnd(cp);
}

static void UnLoad(void) {
  DeleteCopList(cp);
  DeleteBitmap(screen);
}

static void Init(void) {
  CopListActivate(cp);
  EnableDMA(DMAF_BLITTER | DMAF_RASTER | DMAF_BLITHOG);
}

PROFILE(Lines);

static void Render(void) {
  ProfilerStart(Lines);
  {
    short i;

#if LINE == 2
    CpuEdgeSetup(screen, 0);
#elif LINE == 1
    CpuLineSetup(screen, 0);
#elif LINE == 0
    BlitterLineSetup(screen, 0, LINE_OR|LINE_SOLID);
#endif

    for (i = 0; i < screen->width; i += 2) {
#if LINE == 4
      CpuLineOpt(screen->planes[0], screen->bytesPerRow,
                 i, 0, screen->width - 1 - i, screen->height - 1);
#elif LINE == 3
      CpuEdgeOpt(screen->planes[0], screen->bytesPerRow,
                 i, 0, screen->width - 1 - i, screen->height - 1);
#elif LINE == 2
      CpuEdge(i, 0, screen->width - 1 - i, screen->height - 1);
#elif LINE == 1
      CpuLine(i, 0, screen->width - 1 - i, screen->height - 1);
#elif LINE == 0
      BlitterLine(i, 0, screen->width - 1 - i, screen->height - 1);
#endif
    }

    for (i = 0; i < screen->height; i += 2) {
#if LINE == 4
      CpuLineOpt(screen->planes[0], screen->bytesPerRow,
                 0, i, screen->width - 1, screen->height - 1 - i);
#elif LINE == 3
      CpuEdgeOpt(screen->planes[0], screen->bytesPerRow,
                 0, i, screen->width - 1, screen->height - 1 - i);
#elif LINE == 2
      CpuEdge(0, i, screen->width - 1, screen->height - 1 - i);
#elif LINE == 1
      CpuLine(0, i, screen->width - 1, screen->height - 1 - i);
#elif LINE == 0
      BlitterLine(0, i, screen->width - 1, screen->height - 1 - i);
#endif
    }
  }
  ProfilerStop(Lines);
}

EFFECT(Lines, Load, UnLoad, Init, NULL, Render, NULL);
