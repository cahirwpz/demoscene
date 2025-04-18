#include <effect.h>
#include <blitter.h>
#include <copper.h>
#include <gfx.h>
#include <system/memory.h>

#define WIDTH 640
#define HEIGHT 256
#define DEPTH 1

#define SIZE 8
#define SPEED 1

#define COLUMNS (WIDTH / SIZE)
#define LINES   (HEIGHT / SIZE)

static __code short active = 0;

static __code CopListT *cp[2];
static __code CopInsPairT *(*linebpl)[2][HEIGHT];
static __code BitmapT *scroll;

static __code short last_line = -1;
static __code char *line_start;

extern char Text[];

#include "data/text-scroll-font.c"
#include "data/background.c"

static CopListT *MakeCopperList(CopInsPairT **linebpl) {
  CopListT *cp = NewCopList(100 + 3 * HEIGHT);
  CopSetupBitplanes(cp, scroll, DEPTH + background_depth);
  {
    void *ptr = scroll->planes[0];
    short y;

    for (y = 0; y < HEIGHT; y++, ptr += scroll->bytesPerRow) {
      CopWaitSafe(cp, Y(y), 0);
      if ((y & 7) == 0) {
        if (y <= 6 * 8)
          CopSetColor(cp, 1, font_colors[7 - (y >> 3)]);
        if (HEIGHT - y <= 6 * 8)
          CopSetColor(cp, 1, font_colors[7 - ((HEIGHT - y) >> 3)]);
      }
      linebpl[y] = CopMove32(cp, bplpt[0], ptr);
    }
  }
  return CopListFinish(cp);
}

static void Init(void) {
  scroll = NewBitmap(WIDTH, HEIGHT + 16, 1, BM_CLEAR);
  scroll->planes[1] = _background_bpl;

  line_start = Text;

  SetupDisplayWindow(MODE_HIRES, X(0), Y(0), WIDTH, HEIGHT);
  SetupBitplaneFetch(MODE_HIRES, X(0), WIDTH);
  SetupMode(MODE_DUALPF|MODE_HIRES, DEPTH + background_depth);
  LoadColors(font_colors, 0);
  LoadColors(background_colors, IsAGA() ? 16 : 8);

  /* reverse playfield priorities */
  custom->bplcon2 = 0;
  /* AGA fix */
  custom->bplcon3 = BPLCON3_PF2OF0;

  linebpl = MemAlloc(sizeof(CopInsPairT *) * 2 * HEIGHT, MEMF_PUBLIC);
  cp[0] = MakeCopperList((*linebpl)[0]);
  cp[1] = MakeCopperList((*linebpl)[1]);

  CopListActivate(cp[active]);

  EnableDMA(DMAF_RASTER|DMAF_BLITTER);
}

static void Kill(void) {
  CopperStop();
  BlitterStop();

  DeleteCopList(cp[0]);
  DeleteCopList(cp[1]);
  MemFree(linebpl);
  DeleteBitmap(scroll);
}

static void RenderLine(u_char *dst, char *line, short size) {
  short dwidth = scroll->bytesPerRow;
  short swidth = font.bytesPerRow;
  u_char *src = font.planes[0];
  short x = 0;

  while (--size >= 0) {
    short i = (*line++) - 32;
    short j = x++;
    short h = 8;

    if (i < 0)
      continue;

    while (--h >= 0) {
      dst[j] = src[i];
      i += swidth;
      j += dwidth;
    }
  }
}

static void SetupLinePointers(void) {
  CopInsPairT **ins = (*linebpl)[active];
  void *plane = scroll->planes[0];
  int stride = scroll->bytesPerRow;
  int bplsize = scroll->bplSize;
  short y = (int)(frameCount / SPEED + 8) % (short)scroll->height;
  void *start = plane + (short)stride * y;
  void *end = plane + bplsize;
  short n = HEIGHT;

  while (--n >= 0) {
    if (start >= end)
      start -= bplsize;
    CopInsSet32(*ins++, start);
    start += stride;
  }
}

static char *NextLine(char *str) {
  for (; *str; str++)
    if (*str == '\n')
      return ++str;
  return str;
}

static void RenderNextLineIfNeeded(void) {
  Area2D rect = {0, 0, WIDTH, SIZE};
  short s = frameCount / (SPEED * 8);

  if (s > last_line) {
    void *ptr = scroll->planes[0];
    short line_num = (s % (LINES + 2)) * SIZE;
    char *line_end;
    short size;

    line_end = NextLine(line_start);
    size = (line_end - line_start) - 1;

    ptr += line_num * scroll->bytesPerRow;

    rect.y = line_num;
    BitmapClearArea(scroll, &rect);
    WaitBlitter();
    RenderLine(ptr, line_start, min(size, COLUMNS));

    last_line = s;
    line_start = line_end;
  }
}

static void Render(void) {
  SetupLinePointers();
  RenderNextLineIfNeeded();

  CopListRun(cp[active]);
  TaskWaitVBlank();
  active ^= 1;
}

EFFECT(TextScroll, NULL, NULL, Init, Kill, Render, NULL);
