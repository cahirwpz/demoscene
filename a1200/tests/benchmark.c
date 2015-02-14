#include "gfx/pixbuf.h"
#include "gfx/line.h"
#include "std/debug.h"
#include "std/memory.h"
#include "std/random.h"
#include "system/timer.h"
#include "tools/profiling.h"

typedef struct Line {
  int x1, y1;
  int x2, y2;
} LineT;

int main() {
  int n = 100000;
  int i;

  PixBufT *canvas = NewPixBuf(PIXBUF_GRAY, 256, 256);
  LineT *lines = NewTable(LineT, n);

  SetupTimer();

  LOG("Generating %d random lines.", n);

  for (i = 0; i < n; i++) {
    static int r = 0xb3a1773;
    LineT *line = &lines[i];

    line->x1 = RandomInt32(&r) & 255;
    line->x2 = RandomInt32(&r) & 255;
    line->y1 = RandomInt32(&r) & 255;
    line->y2 = RandomInt32(&r) & 255;
  }

  StartProfiling();

  PROFILE (DrawLineUnsafe)
    for (i = 0; i < n; i++) {
      LineT *line = &lines[i];

      DrawLineUnsafe(canvas, line->x1, line->y1, line->x2, line->y2);
    }

  PROFILE (DrawLineAA)
    for (i = 0; i < n; i++) {
      LineT *line = &lines[i];

      DrawLineAA(canvas, line->x1, line->y1, line->x2, line->y2);
    }

  StopProfiling();

  MemUnref(lines);
  MemUnref(canvas);

  KillTimer();

  return 0;
}
