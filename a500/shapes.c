#include "2d.h"
#include "blitter.h"
#include "coplist.h"
#include "memory.h"
#include "interrupts.h"
#include "file.h"
#include "reader.h"

typedef struct Polygon {
  UWORD vertices;
  UWORD index;
} PolygonT;

typedef struct Shape {
  UWORD points;
  UWORD polygons;
  UWORD polygonVertices;

  PointT *origPoint;
  PointT *viewPoint;
  UBYTE *viewPointFlags;
  PolygonT *polygon;
  UWORD *polygonVertex;
} ShapeT;

ShapeT *NewShape(UWORD points, UWORD polygons) {
  ShapeT *shape = AllocMemSafe(sizeof(ShapeT), MEMF_PUBLIC|MEMF_CLEAR);

  shape->points = points;
  shape->polygons = polygons;

  shape->origPoint = AllocMemSafe(sizeof(PointT) * points, MEMF_PUBLIC);
  shape->viewPoint = AllocMemSafe(sizeof(PointT) * points, MEMF_PUBLIC);
  shape->viewPointFlags = AllocMemSafe(points, MEMF_PUBLIC);
  shape->polygon = AllocMemSafe(sizeof(PolygonT) * polygons, MEMF_PUBLIC);

  return shape;
}

void DeleteShape(ShapeT *shape) {
  if (shape->polygonVertex)
    FreeMem(shape->polygonVertex, sizeof(UWORD) * shape->polygonVertices);
  FreeMem(shape->polygon, sizeof(PolygonT) * shape->polygons);
  FreeMem(shape->viewPointFlags, shape->points);
  FreeMem(shape->viewPoint, sizeof(PointT) * shape->points);
  FreeMem(shape->origPoint, sizeof(PointT) * shape->points);
  FreeMem(shape, sizeof(ShapeT));
}

ShapeT *LoadShape(char *filename) {
  char *file = ReadFile(filename, MEMF_PUBLIC);
  char *data = file;
  ShapeT *shape = NULL;
  WORD i, j, points, polygons;

  if (!file)
    return NULL;
  
  if (ReadNumber(&data, &points) && ReadNumber(&data, &polygons)) {
    shape = NewShape(points, polygons);

    for (i = 0; i < shape->points; i++) {
      if (!ReadNumber(&data, &shape->origPoint[i].x) ||
          !ReadNumber(&data, &shape->origPoint[i].y))
        goto error;
    }

    /* Calculate size of polygonVertex array. */
    {
      char *origData = data;

      for (i = 0; i < shape->polygons; i++) {
        WORD n, tmp;

        if (!ReadNumber(&data, &n))
          goto error;

        shape->polygonVertices += n;

        while (n--) {
          if (!ReadNumber(&data, &tmp))
            goto error;
        }
      }

      data = origData;
    }

    shape->polygonVertex =
      AllocMemSafe(sizeof(UWORD) * shape->polygonVertices, MEMF_PUBLIC);

    for (i = 0, j = 0; i < shape->polygons; i++) {
      UWORD n;

      if (!ReadNumber(&data, &n))
        goto error;

      Log("Polygon %ld at %ld:", (LONG)i, (LONG)j);

      shape->polygon[i].vertices = n;
      shape->polygon[i].index = j;

      while (n--) {
        UWORD tmp;

        if (!ReadNumber(&data, &tmp))
          goto error;

        shape->polygonVertex[j++] = tmp;
        Log(" %ld", (LONG)tmp);
      }

      Log("\n");
    }

    FreeAutoMem(file);
    return shape;
  }

error:
  DeleteShape(shape);
  FreeAutoMem(file);
  return NULL;
}

static ShapeT *shape;
static BitmapT *screen;
static CopInsT *bplptr[5];
static CopListT *cp;
static WORD plane, planeC;
static BoxT clipBox = { 80, 64, 240 - 1, 192 - 1 };

void Load() {
  screen = NewBitmap(320, 256, 5, FALSE);
  shape = LoadShape("data/boxes.2d");
  cp = NewCopList(100);

  CopInit(cp);
  CopMakePlayfield(cp, bplptr, screen);
  CopMakeDispWin(cp, 0x81, 0x2c, screen->width, screen->height);
  {
    UWORD i, j = 2;

    CopSetRGB(cp, 0, 0x000);
    CopSetRGB(cp, 1, 0x111);
    for (i = 0; i < 2; i++)
      CopSetRGB(cp, j++, 0x222);
    for (i = 0; i < 4; i++)
      CopSetRGB(cp, j++, 0x444);
    for (i = 0; i < 8; i++)
      CopSetRGB(cp, j++, 0x888);
    for (i = 0; i < 16; i++)
      CopSetRGB(cp, j++, 0xfff);
  }
  CopEnd(cp);

  plane = screen->depth - 1;
  planeC = 0;
}

void Kill() {
  DeleteShape(shape);
  DeleteCopList(cp);
  DeleteBitmap(screen);
}

static __regargs BOOL CheckInside(PointT *p, WORD limit, UWORD plane) {
  if (plane & PF_LEFT)
    return (p->x >= limit);
  if (plane & PF_RIGHT)
    return (p->x < limit);
  if (plane & PF_TOP)
    return (p->y >= limit);
  if (plane & PF_BOTTOM)
    return (p->y < limit);
  return FALSE;
}

static __regargs void ClipEdge(PointT *o, PointT *s, PointT *e,
                               WORD limit, UWORD plane) 
{
  WORD dx = s->x - e->x;
  WORD dy = s->y - e->y;

  if (plane & (PF_LEFT | PF_RIGHT)) {
    o->x = limit;
    o->y = e->y + div16(dy * (limit - e->x), dx);
  } 

  if (plane & (PF_TOP | PF_BOTTOM)) {
    o->x = e->x + div16(dx * (limit - e->y), dy);
    o->y = limit;
  }
}

__regargs UWORD SutherlandHodgman(PointT *S, PointT *O,
                                  UWORD n, WORD limit, UWORD plane)
{
  PointT *E = S + 1;

  BOOL S_inside = CheckInside(S, limit, plane);
  BOOL needClose = TRUE;
  UWORD m = 0;

  if (S_inside) {
    needClose = FALSE;
    O[m++] = *S;
  }

  while (--n) {
    BOOL E_inside = CheckInside(E, limit, plane);

    if (S_inside && E_inside) {
      O[m++] = *E;
    } else if (S_inside && !E_inside) {
      ClipEdge(&O[m++], S, E, limit, plane);
    } else if (!S_inside && E_inside) {
      ClipEdge(&O[m++], E, S, limit, plane);
      O[m++] = *E;
    }

    S_inside = E_inside;
    S++; E++;
  }

  if (needClose)
    O[m++] = *O;

  return m;
}

static PointT tmpPoint0[16];
static PointT tmpPoint1[16];

static void DrawShape(ShapeT *shape) {
  PointT *point = shape->viewPoint;
  PolygonT *polygon = shape->polygon;
  UBYTE *flags = shape->viewPointFlags;
  UWORD *vertex = shape->polygonVertex;
  UWORD ps = shape->polygons;

  PointsInsideBox(point, flags, shape->points, &clipBox);

  while (ps--) {
    UWORD i, j;
    UWORD n = polygon->vertices;
    UWORD m = n + 1;
    UBYTE clip = 0;
    UBYTE outside = 0xff;

    for (j = 0, i = polygon->index; j < n; j++) {
      UWORD k = vertex[i++];

      clip |= flags[k];
      outside &= flags[k];
      tmpPoint0[j] = point[k];
    }
    tmpPoint0[j] = point[vertex[i - n]];

    if (!outside) {
      if (clip) {
        m = SutherlandHodgman(tmpPoint0, tmpPoint1, m, clipBox.minX, PF_LEFT);
        m = SutherlandHodgman(tmpPoint1, tmpPoint0, m, clipBox.minY, PF_TOP);
        m = SutherlandHodgman(tmpPoint0, tmpPoint1, m, clipBox.maxX, PF_RIGHT);
        m = SutherlandHodgman(tmpPoint1, tmpPoint0, m, clipBox.maxY, PF_BOTTOM);
      }

      for (j = 0; j < m - 1; j++) {
        WaitBlitter();
        BlitterLine(screen, plane, LINE_EOR, ONEDOT,
                    tmpPoint0[j].x, tmpPoint0[j].y, tmpPoint0[j+1].x, tmpPoint0[j+1].y);
      }
    }

    polygon++;
  }
}

static ULONG frameCount = 0;

__interrupt_handler void IntLevel3Handler() {
  if (custom->intreqr & INTF_VERTB)
    frameCount++;

  custom->intreq = INTF_LEVEL3;
  custom->intreq = INTF_LEVEL3;
}

static BOOL Loop() {
  UWORD i, a = (frameCount * 8) & 0x1ff;
  View2D t;

  BlitterClear(screen, plane);

  Identity2D(&t);
  Rotate2D(&t, frameCount);
  Scale2D(&t, 288 + sincos[a].sin / 2, 288 + sincos[a].cos / 2);
  Translate2D(&t, screen->width / 2, screen->height / 2);
  Transform2D(&t, shape->viewPoint, shape->origPoint, shape->points);

  DrawShape(shape);

  WaitBlitter();
  BlitterFill(screen, plane);
  WaitBlitter();

  WaitVBlank();

  for (i = 0; i < screen->depth; i++) {
    WORD j = plane + i;

    if (j >= screen->depth)
      j -= screen->depth;

    CopInsSet32(bplptr[i], screen->planes[j]);
  }

  if (planeC & 1) {
    plane++;
    if (plane >= screen->depth)
      plane -= screen->depth;
  }
  planeC ^= 1;

  return !LeftMouseButton();
}

void Main() {
  InterruptVector->IntLevel3 = IntLevel3Handler;
  custom->intena = INTF_SETCLR | INTF_VERTB;
  
  CopListActivate(cp);
  custom->dmacon = DMAF_SETCLR | DMAF_BLITTER | DMAF_RASTER;

  while (Loop());
}
