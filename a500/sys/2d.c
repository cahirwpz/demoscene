#include "2d.h"
#include "memory.h"
#include "io.h"
#include "reader.h"
#include "fx.h"

__regargs void LoadIdentity2D(Matrix2D *M) {
  M->m00 = fx12f(1.0);
  M->m01 = 0;
  M->x = 0;

  M->m10 = 0;
  M->m11 = fx12f(1.0);
  M->y = 0;
}

__regargs void Translate2D(Matrix2D *M, WORD x, WORD y) {
  M->x += x;
  M->y += y;
}

__regargs void Scale2D(Matrix2D *M, WORD sx, WORD sy) {
  M->m00 = normfx(M->m00 * sx);
  M->m01 = normfx(M->m01 * sy);
  M->m10 = normfx(M->m10 * sx);
  M->m11 = normfx(M->m11 * sy);
}

__regargs void Rotate2D(Matrix2D *M, WORD a) {
  WORD sin = SIN(a);
  WORD cos = COS(a);
  WORD m00 = M->m00;
  WORD m01 = M->m01;
  WORD m10 = M->m10;
  WORD m11 = M->m11;

  M->m00 = normfx(m00 * cos - m01 * sin);
  M->m01 = normfx(m00 * sin + m01 * cos);
  M->m10 = normfx(m10 * cos - m11 * sin);
  M->m11 = normfx(m10 * sin + m11 * cos);
}

__regargs void Transform2D(Matrix2D *M, Point2D *out, Point2D *in, WORD n) {
  WORD *dst = (WORD *)out;
  WORD *src = (WORD *)in;
  WORD *v = (WORD *)M;
  WORD m00 = *v++;
  WORD m01 = *v++;
  WORD mx  = *v++;
  WORD m10 = *v++;
  WORD m11 = *v++;
  WORD my  = *v++;

  while (--n >= 0) {
    WORD x = *src++;
    WORD y = *src++;

    *dst++ = normfx(m00 * x + m01 * y) + mx;
    *dst++ = normfx(m10 * x + m11 * y) + my;
  }
}

typedef struct {
  const char *name;
  BOOL (*func)(char **, ShapeT *);
} ParserT;

static BOOL ParseOrigin(char **data, ShapeT *shape) {
  if (!(ReadShort(data, &shape->origin.x) &&
        ReadShort(data, &shape->origin.y) &&
        EndOfLine(data)))
    return FALSE;
  Log("[2D] Origin is (%ld, %ld)\n", 
      (LONG)shape->origin.x, (LONG)shape->origin.y);
  return TRUE;
}

static BOOL ParsePoints(char **data, ShapeT *shape) {
  Point2D *point;
  WORD n;

  if (!(ReadShort(data, &n) && EndOfLine(data)))
    return FALSE;

  Log("[2D] Shape has %ld points\n", (LONG)n);

  shape->points = n;
  shape->origPoint = MemAlloc(sizeof(Point2D) * n, MEMF_PUBLIC);
  shape->viewPoint = MemAlloc(sizeof(Point2D) * n, MEMF_PUBLIC);
  shape->viewPointFlags = MemAlloc(n, MEMF_PUBLIC);

  point = shape->origPoint;

  while (NextLine(data) && !MatchString(data, "@end") && n > 0) {
    WORD x, y;

    if (!(ReadShort(data, &x) && ReadShort(data, &y) && EndOfLine(data)))
      return FALSE;

    point->x = (x - shape->origin.x) * 16;
    point->y = (y - shape->origin.y) * 16;
    point++, n--;
  }

  return (n == 0);
}

static BOOL ParsePolygons(char **data, ShapeT *shape) {
  IndexListT **polygons;
  UBYTE *polygonFlags;
  WORD *index;
  WORD k = 0, n;

  if (!(ReadShort(data, &n) && EndOfLine(data)))
    return FALSE;

  Log("[2D] Shape has %ld polygons\n", (LONG)n);

  shape->polygons = n;
  shape->polygon = MemAlloc(sizeof(IndexListT *) * (n + 1) + 
                            sizeof(WORD) * (n * 2 + shape->points),
                            MEMF_PUBLIC);
  shape->polygonFlags = MemAlloc(sizeof(UBYTE) * n, MEMF_PUBLIC);

  polygons = shape->polygon;
  polygonFlags = shape->polygonFlags;
  index = (WORD *)&shape->polygon[n + 1];

  while (NextLine(data) && !MatchString(data, "@end") && n > 0) {
    WORD count, first = k;

    if (!(ReadByte(data, polygonFlags++) && ReadShort(data, &count) &&
          EndOfLine(data)))
      return FALSE;

    *polygons++ = (IndexListT *)index;

    *index++ = count;
    do {
      *index++ = k++;
    } while (--count);
    *index++ = first;

    n--;
  }

  return (n == 0);
}

static ParserT TopLevelParser[] = {
  { "@origin", &ParseOrigin},
  { "@pnts", &ParsePoints },
  { "@pols", &ParsePolygons },
  { NULL, NULL }
};

__regargs ShapeT *LoadShape(char *filename) {
  char *file = LoadFile(filename, MEMF_PUBLIC);
  char *data = file;
  ShapeT *shape = MemAlloc(sizeof(ShapeT), MEMF_PUBLIC|MEMF_CLEAR);

  Log("[2D] Parsing '%s' file\n", filename);

  while (NextLine(&data)) {
    ParserT *parser = TopLevelParser;
    
    for (; parser->name; parser++) {
      if (!MatchString(&data, parser->name))
        continue;
      if (parser->func(&data, shape))
        break;
      Log("[2D] Parse error at position %ld!\n", (LONG)(data - file));
      DeleteShape(shape);
      MemFree(file);
      return NULL;
    }
  }

  MemFree(file);
  return shape;
}

__regargs void DeleteShape(ShapeT *shape) {
  if (shape) {
    MemFree(shape->polygonFlags);
    MemFree(shape->polygon);
    MemFree(shape->viewPointFlags);
    MemFree(shape->viewPoint);
    MemFree(shape->origPoint);
    MemFree(shape);
  }
}
