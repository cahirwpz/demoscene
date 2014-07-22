#include "blitter.h"
#include "coplist.h"
#include "file.h"
#include "memory.h"
#include "print.h"
#include "3d.h"

#define X(x) ((x) + 0x81)
#define Y(y) ((y) + 0x2c)

static Object3D object;
static View3D view3d;

static BitmapT *screen;
static CopListT *cp;

void Load() {
  screen = NewBitmap(320, 256, 1, FALSE);
  cp = NewCopList(100);

  {
    static VertexT vertex[8] = {
      { -100, -100, -100 },
      {  100, -100, -100 },
      {  100,  100, -100 },
      { -100,  100, -100 },
      { -100, -100,  100 },
      {  100, -100,  100 },
      {  100,  100,  100 },
      { -100,  100,  100 }
    };

    static EdgeT edge[12] = {
      { 0, 1 },
      { 1, 2 },
      { 2, 3 },
      { 3, 0 },
      { 4, 5 },
      { 5, 6 },
      { 6, 7 },
      { 7, 4 },
      { 0, 4 },
      { 1, 5 },
      { 2, 6 },
      { 3, 7 }
    };

    static PointT point[8];
    static UBYTE pointFlags[8];
    static LineT line[12];
    static UBYTE lineFlags[12];

    object.nVertex = 8;
    object.nEdge = 12;
    object.vertex = vertex;
    object.edge = edge;
    object.point = point;
    object.pointFlags = pointFlags;
    object.line = line;
    object.lineFlags = lineFlags;
  }
}

void Kill() {
  DeleteCopList(cp);
  DeleteBitmap(screen);
}

#if 0
static WORD lines[256];
static WORD pixel[8] = { 128, 64, 32, 16, 8, 4, 2, 1 };

static void CalculateLines() {
  WORD i, j;
  WORD l = screen->width / 8;

  for (i = 0, j = 0; i < 256; i++, j += l)
    lines[i] = j;
}

static void ClearPoints() {
  UBYTE *plane = screen->planes[0];
  WORD *coords = (WORD *)point[buffer ^ 1];
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
  WORD *coords = (WORD *)point[buffer];
  WORD i;

  for (i = 0; i < 8; i++) {
    WORD x = *coords++;
    WORD y = *coords++;

    if (x < 0 || y < 0 || x >= 320 || y >= 256)
     continue;

    plane[lines[y] + (x >> 3)] |= pixel[x & 7];
  }
}
#endif

typedef struct {
  WORD p, q;
} LBEdgeT;

static inline WORD div16(LONG a, WORD b) {
  asm("divs %1,%0"
      : "+d" (a)
      : "d" (b));
  return a;
}

static BOOL LiangBarsky(LineT *line) {
  static LBEdgeT edge[4];

  WORD t0 = 0;
  WORD t1 = 0x1000;
  WORD xdelta = line->x2 - line->x1;
  WORD ydelta = line->y2 - line->y1;
  WORD i;

  edge[0].p = -xdelta;
  edge[0].q = line->x1;
  edge[1].p = xdelta;
  edge[1].q = 319 - line->x1;
  edge[2].p = -ydelta;
  edge[2].q = line->y1;
  edge[3].p = ydelta;
  edge[3].q = 255 - line->y1;

  for (i = 0; i < 4; i++) {
    WORD p, r;
   
    p = edge[i].p;

    if (p == 0) {
      i++;
      continue;
    }

    r = div16(edge[i].q * 0x1000, p);

    if (p < 0) {
      if (r > t1)
        return FALSE;

      if (r > t0)
        t0 = r;
    } else {
      if (r < t0)
        return FALSE;

      if (r < t1)
        t1 = r;
    }
  }

  if (t0 > 0) {
    line->x1 += (t0 * xdelta + 0x800) / 0x1000;
    line->y1 += (t0 * ydelta + 0x800) / 0x1000;
  }

  if (t1 < 0x1000) {
    t1 -= 0x1000;
    line->x2 += (t1 * xdelta + 0x800) / 0x1000;
    line->y2 += (t1 * ydelta + 0x800) / 0x1000;
  }

  return TRUE;
}

void ClipEdges(Object3D *object) {
  WORD n = object->nEdge;
  PointT *point = object->point;
  UBYTE *pointFlags = object->pointFlags;
  EdgeT *edge = object->edge;
  LineT *line = object->line;
  UBYTE *lineFlags = object->lineFlags;

  do {
    UWORD i1 = edge->p1;
    UWORD i2 = edge->p2;
    BOOL outside = pointFlags[i1] & pointFlags[i2];
    BOOL clip = pointFlags[i1] | pointFlags[i2];

    line->x1 = point[i1].x;
    line->y1 = point[i1].y;
    line->x2 = point[i2].x;
    line->y2 = point[i2].y;

    if (!outside && clip)
      outside = !LiangBarsky(line);

    edge++;
    line++;
    *lineFlags++ = outside;
  } while (--n);
}

static void DrawObject() {
  LineT *line = object.line;
  UBYTE *lineFlags = object.lineFlags;
  WORD n = object.nEdge;

  do {
    WaitBlitter();
    if (!*lineFlags++)
      BlitterLine(screen, 0, LINE_OR, 0, line->x1, line->y1, line->x2, line->y2);
    line++;
  } while (--n);
}

void Main() {
  CopInit(cp);
  CopMakePlayfield(cp, NULL, screen);
  CopMakeDispWin(cp, X(0), Y(0), screen->width, screen->height);
  CopSetRGB(cp, 0, 0x000);
  CopSetRGB(cp, 1, 0xfff);
  CopEnd(cp);

  CopListActivate(cp);

  custom->dmacon = DMAF_SETCLR | DMAF_BLITTER | DMAF_RASTER;

  view3d.viewerX = 100;
  view3d.viewerY = 100;
  view3d.viewerZ = 300;

  while (!LeftMouseButton()) {
    view3d.rotateX++;
    view3d.rotateY++;
    view3d.rotateZ++;

    WaitLine(Y(0));

    custom->color[0] = 0x00f;
    CalculateView3D(&view3d);
    TransformVertices(&view3d, &object);
    ClipEdges(&object);
    custom->color[0] = 0x000;

    WaitLine(Y(200));

    custom->color[0] = 0xf00;

    WaitBlitter();
    BlitterClear(screen, 0);

    custom->color[0] = 0x0f0;
    DrawObject();
    custom->color[0] = 0x000;

#if 0
    WaitVBlank();
#endif
  }
}
