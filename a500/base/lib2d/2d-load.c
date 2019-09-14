#include "2d.h"
#include "memory.h"
#include "io.h"
#include "reader.h"

typedef struct {
  const char *name;
  bool (*func)(char **, ShapeT *);
} ParserT;

static bool ParseOrigin(char **data, ShapeT *shape) {
  if (!(ReadShort(data, &shape->origin.x) &&
        ReadShort(data, &shape->origin.y) &&
        EndOfLine(data)))
    return false;
  Log("[2D] Origin is (%d, %d)\n", shape->origin.x, shape->origin.y);
  return true;
}

static bool ParsePoints(char **data, ShapeT *shape) {
  Point2D *point;
  short n;

  if (!(ReadShort(data, &n) && EndOfLine(data)))
    return false;

  Log("[2D] Shape has %d points\n", n);

  shape->points = n;
  shape->origPoint = MemAlloc(sizeof(Point2D) * n, MEMF_PUBLIC);
  shape->viewPoint = MemAlloc(sizeof(Point2D) * n, MEMF_PUBLIC);
  shape->viewPointFlags = MemAlloc(n, MEMF_PUBLIC);

  point = shape->origPoint;

  while (NextLine(data) && !MatchString(data, "@end") && n > 0) {
    short x, y;

    if (!(ReadShort(data, &x) && ReadShort(data, &y) && EndOfLine(data)))
      return false;

    point->x = (x - shape->origin.x) * 16;
    point->y = (y - shape->origin.y) * 16;
    point++, n--;
  }

  return (n == 0);
}

static bool ParsePolygons(char **data, ShapeT *shape) {
  IndexListT **polygons;
  u_char *polygonFlags;
  short *index;
  short k = 0, n;

  if (!(ReadShort(data, &n) && EndOfLine(data)))
    return false;

  Log("[2D] Shape has %d polygons\n", n);

  shape->polygons = n;
  shape->polygon = MemAlloc(sizeof(IndexListT *) * (n + 1) + 
                            sizeof(short) * (n * 2 + shape->points),
                            MEMF_PUBLIC|MEMF_CLEAR);
  shape->polygonFlags = MemAlloc(sizeof(u_char) * n, MEMF_PUBLIC);

  polygons = shape->polygon;
  polygonFlags = shape->polygonFlags;
  index = (short *)&shape->polygon[n + 1];

  while (NextLine(data) && !MatchString(data, "@end") && n > 0) {
    short count, first = k;

    if (!(ReadByteU(data, polygonFlags++) && ReadShort(data, &count) &&
          EndOfLine(data)))
      return false;

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
  { "@origin", &ParseOrigin },
  { "@pnts", &ParsePoints },
  { "@pols", &ParsePolygons },
  { NULL, NULL }
};

__regargs ShapeT *LoadShape(const char *filename) {
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
      Log("[2D] Parse error at position %ld!\n", (ptrdiff_t)(data - file));
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
