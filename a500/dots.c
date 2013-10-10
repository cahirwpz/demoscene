#include "blitter.h"
#include "coplist.h"
#include "file.h"
#include "memory.h"
#include "print.h"
#include "3d.h"

#define X(x) ((x) + 0x81)
#define Y(y) ((y) + 0x2c)

static VertexT cube3d[8] = {
  { -100, -100, -100 },
  {  100, -100, -100 },
  {  100,  100, -100 },
  { -100,  100, -100 },
  { -100, -100,  100 },
  {  100, -100,  100 },
  {  100,  100,  100 },
  { -100,  100,  100 }
};

static WORD buffer = 0;
static PointT cube2d[2][8];
static View3D view3d;
static WORD alpha = 0, beta = 0, gamma = 0;
static WORD lines[256];
static WORD pixel[8] = { 128, 64, 32, 16, 8, 4, 2, 1 };

static BitmapT *screen;
static CopListT *cp;

void Load() {

  screen = NewBitmap(320, 256, 1, FALSE);
  cp = NewCopList(100);

  {
    WORD i, j;
    WORD l = screen->width / 8;

    for (i = 0, j = 0; i < 256; i++, j += l)
      lines[i] = j;
  }
}

void Kill() {
  DeleteCopList(cp);
  DeleteBitmap(screen);
}

static void ClearPoints() {
  UBYTE *plane = screen->planes[0];
  WORD *coords = (WORD *)cube2d[buffer ^ 1];
  WORD i;

  for (i = 0; i < 8; i++) {
    WORD x = *coords++;
    WORD y = *coords++;

    if (x < 0 || y < 0 || x >= 320 || y >= 256)
     continue;

    plane[lines[y] + (x >> 3)] = 0;
  }
}

static void DrawPoints() {
  UBYTE *plane = screen->planes[0];
  WORD *coords = (WORD *)cube2d[buffer];
  WORD i;

  for (i = 0; i < 8; i++) {
    WORD x = *coords++;
    WORD y = *coords++;

    if (x < 0 || y < 0 || x >= 320 || y >= 256)
     continue;

    plane[lines[y] + (x >> 3)] |= pixel[x & 7];
  }
}

void Main() {
  CopInit(cp);

  CopMove16(cp, bplcon0, BPLCON0_BPU(screen->depth) | BPLCON0_COLOR);
  CopMove16(cp, bplcon1, 0);
  CopMove16(cp, bplcon2, 0);
  CopMove32(cp, bplpt[0], screen->planes[0]);

  CopMove16(cp, ddfstrt, 0x38);
  CopMove16(cp, ddfstop, 0xd0);
  CopMakeDispWin(cp, X(0), Y(0), screen->width, screen->height);
  CopMove16(cp, color[0], 0x000);
  CopMove16(cp, color[1], 0xfff);

  CopEnd(cp);
  CopListActivate(cp);

  custom->dmacon = DMAF_SETCLR | DMAF_RASTER | DMAF_MASTER;

  view3d.viewerX = 160;
  view3d.viewerY = 100;
  view3d.viewerZ = 100;

  while (!LeftMouseButton()) {
    alpha++;
    beta++;
    gamma++;

    WaitLine(Y(0));

    custom->color[0] = 0x00f;
    CalculateView3D(&view3d, alpha, beta, gamma);
    TransformVertices(&view3d, cube3d, cube2d[buffer], 8);
    custom->color[0] = 0x000;

    WaitLine(Y(256));

    custom->color[0] = 0x0f0;
    ClearPoints();
    DrawPoints();
    buffer ^= 1;
    custom->color[0] = 0x000;

    WaitVBlank();
  }
}
