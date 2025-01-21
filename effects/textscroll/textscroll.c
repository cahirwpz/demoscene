#include <effect.h>
#include <blitter.h>
#include <copper.h>
#include <gfx.h>
#include <system/memory.h>

#define WIDTH 640
#define HEIGHT 256
#define DEPTH 1

#define SIZE 8

#define COLUMNS (WIDTH / SIZE)
#define LINES   (HEIGHT / SIZE)

static short active = 0;

static CopListT *cp[2];
static CopInsPairT *linebpl[2][HEIGHT];
static BitmapT *scroll;

static short last_line = -1;
static char *line_start;

extern char binary_data_text_scroll_txt_start[];
#define text binary_data_text_scroll_txt_start

#include "data/text-scroll-font.c"

static CopListT *MakeCopperList(short n) {
  CopListT *cp = NewCopList(100 + 3 * HEIGHT);
  CopSetupBitplanes(cp, scroll, DEPTH);
  {
    u_short i;
    void *ptr = scroll->planes[0];

    for (i = 0; i < HEIGHT; i++, ptr += scroll->bytesPerRow) {
      CopWaitSafe(cp, Y(i), 0);
      linebpl[n][i] = CopMove32(cp, bplpt[0], ptr);
    }
  }
  return CopListFinish(cp);
}

static void Init(void) {
  scroll = NewBitmap(WIDTH, HEIGHT + 16, 1, BM_CLEAR);

  EnableDMA(DMAF_BLITTER);
  BitmapClear(scroll);

  line_start = text;

  SetupPlayfield(MODE_HIRES, DEPTH, X(0), Y(0), WIDTH, HEIGHT);
  LoadColors(font_colors, 0);

  cp[0] = MakeCopperList(0);
  cp[1] = MakeCopperList(1);

  CopListActivate(cp[active]);

  EnableDMA(DMAF_RASTER);
}

static void Kill(void) {
  DeleteCopList(cp[0]);
  DeleteCopList(cp[1]);
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
  CopInsPairT **ins = linebpl[active];
  void *plane = scroll->planes[0];
  int stride = scroll->bytesPerRow;
  int bplsize = scroll->bplSize;
  short y = (int)(frameCount / 2 + 8) % (short)scroll->height;
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
  short s = frameCount / 16;

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
