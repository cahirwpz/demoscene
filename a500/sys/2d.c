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

#define TOPLEVEL 0
#define PNTS 1
#define POLS 2

__regargs ShapeT *LoadShape(char *filename) {
  char *file = LoadFile(filename, MEMF_PUBLIC);
  char *data = file;
  ShapeT *shape = MemAlloc(sizeof(ShapeT), MEMF_PUBLIC|MEMF_CLEAR);

  WORD origin_x = 0, origin_y = 0;
  WORD points = 0, polygons = 0;
  WORD i = 0, j = 0, k = 0;
  WORD state = TOPLEVEL;

  Log("[2D] Parsing '%s' shape\n", filename);

  while (NextLine(&data)) {
    if (state == TOPLEVEL) {
      if (MatchString(&data, "@origin")) {
        if (!(ReadShort(&data, &origin_x) &&
              ReadShort(&data, &origin_y) &&
              EndOfLine(&data)))
          goto error;
        Log("[2D] Origin is (%ld, %ld)\n", (LONG)origin_x, (LONG)origin_y);
      } else if (MatchString(&data, "@pnts")) {
        if (!(ReadShort(&data, &points) && EndOfLine(&data)))
          goto error;
        Log("[2D] Shape has %ld points\n", (LONG)points);

        state = PNTS, i = 0;

        shape->points = points;
        shape->origPoint = MemAlloc(sizeof(Point2D) * points, MEMF_PUBLIC);
        shape->viewPoint = MemAlloc(sizeof(Point2D) * points, MEMF_PUBLIC);
        shape->viewPointFlags = MemAlloc(points, MEMF_PUBLIC);
      } else if (MatchString(&data, "@pols")) {
        if (!(ReadShort(&data, &polygons) && EndOfLine(&data)))
          goto error;
        Log("[2D] Shape has %ld polygons\n", (LONG)polygons);

        state = POLS, i = 0, j = 0, k = 0;

        shape->polygons = polygons;
        shape->polygon = 
          MemAlloc(sizeof(IndexListT *) * polygons, MEMF_PUBLIC);
        shape->polygonFlags = 
          MemAlloc(sizeof(UBYTE) * polygons, MEMF_PUBLIC);
        shape->polygonData =
          MemAlloc(sizeof(WORD) * (polygons * 2 + points), MEMF_PUBLIC);
      }
    } else if (state == PNTS) {
      if (MatchString(&data, "@end")) {
        if (i != points)
          goto error;
        state = TOPLEVEL;
      } else {
        WORD x, y;

        if (!(ReadShort(&data, &x) && ReadShort(&data, &y) && EndOfLine(&data)))
          goto error;

        shape->origPoint[i].x = (x - origin_x) * 16;
        shape->origPoint[i].y = (y - origin_y) * 16;
        i++;
      }
    } else if (state == POLS) {
      if (MatchString(&data, "@end")) {
        if (i != polygons)
          goto error;
        state = TOPLEVEL;
      } else {
        WORD flags, n, first = k;

        if (!(ReadShort(&data, &flags) &&
              ReadShort(&data, &n) &&
              EndOfLine(&data)))
          goto error;

        shape->polygon[i] = (IndexListT *)&shape->polygonData[j];
        shape->polygonFlags[i] = flags;
        shape->polygonData[j++] = n;
        do {
          shape->polygonData[j++] = k++;
        } while (--n);
        shape->polygonData[j++] = first;
        i++;
      }
    }
  }

  return shape;

error:
  Log("[2D] Parse error at position %ld!\n", (LONG)(data - file));
  DeleteShape(shape);
  MemFree(file);
  return NULL;
}

__regargs void DeleteShape(ShapeT *shape) {
  if (shape) {
    MemFree(shape->polygonFlags);
    MemFree(shape->polygonData);
    MemFree(shape->polygon);
    MemFree(shape->viewPointFlags);
    MemFree(shape->viewPoint);
    MemFree(shape->origPoint);
    MemFree(shape);
  }
}
