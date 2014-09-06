#include "blitter.h"
#include "coplist.h"
#include "3d.h"
#include "fx.h"
#include "iff.h"
#include "ffp.h"
#include "memory.h"

#define X(x) ((x) + 0x81)
#define Y(y) ((y) + 0x2c)

#define WIDTH  320
#define HEIGHT 256

static Object3D *cube;

static CopListT *cp;
static BitmapT *screen[2];
static UWORD active = 0;
static CopInsT *bplptr[8];

#define ID_LWOB MAKE_ID('L', 'W', 'O', 'B')
#define ID_LWO2 MAKE_ID('L', 'W', 'O', '2')
#define ID_PNTS MAKE_ID('P', 'N', 'T', 'S')
#define ID_POLS MAKE_ID('P', 'O', 'L', 'S')

__regargs Object3D *LoadLWO(char *filename, LONG scale) {
  Object3D *obj = NULL;
  IffFileT iff;

  if (OpenIff(&iff, filename)) {
    if (iff.header.type == ID_LWOB || iff.header.type == ID_LWO2) {
      FLOAT *pnts = NULL;
      WORD *pols = NULL;
      LONG pntsLength = 0;
      LONG polsLength = 0;

      while (ParseChunk(&iff)) {
        switch (iff.chunk.type) {
          case ID_PNTS:
            pntsLength = iff.chunk.length;
            pnts = AllocMemSafe(pntsLength, MEMF_PUBLIC);
            ReadChunk(&iff, pnts);
            break;

          case ID_POLS:
            polsLength = iff.chunk.length;
            pols = AllocMemSafe(polsLength, MEMF_PUBLIC);
            ReadChunk(&iff, pols);
            break;

          default:
            SkipChunk(&iff);
            break;
        }
      }

      {
        UWORD points = pntsLength / 12;
        UWORD polygons = 0;
        UWORD polygonVertices = 0;

        LONG i = 0;
        LONG n = polsLength / 2;

        if (iff.header.type == ID_LWOB) {
          while (i < n) {
            WORD vertices = pols[i++];
            polygonVertices += vertices;
            polygons++;
            i += vertices + 1;
          }
        } else {
          i += 2;
          while (i < n) {
            WORD vertices = pols[i++];
            polygonVertices += vertices;
            polygons++;
            i += vertices;
          }
        }

        obj = NewObject3D(points, polygons);

        Log("File '%s' has %ld points and %ld polygons.\n", 
            filename, (LONG)points, (LONG)polygons);

        /* Process points. */
        {
          FLOAT s = SPFlt(scale * 16);

          for (i = 0; i < points; i++) {
            obj->point[i].x = SPFix(SPMul(SPFieee(pnts[i * 3 + 0]), s));
            obj->point[i].y = SPFix(SPMul(SPFieee(pnts[i * 3 + 1]), s));
            obj->point[i].z = SPFix(SPMul(SPFieee(pnts[i * 3 + 2]), s));
          }

          FreeMem(pnts, pntsLength);
        }

        /* Process polygons. */
        {
          WORD *polygonVertex = AllocMemSafe(sizeof(UWORD) * polygonVertices,
                                             MEMF_PUBLIC);
          WORD p = 0, j = 0;

          i = 0;
          n = polsLength / 2;

          if (iff.header.type == ID_LWOB) {
            while (i < n) {
              WORD vertices = pols[i++];
              obj->polygon[p].vertices = vertices;
              obj->polygon[p].index = j;
              while (--vertices >= 0)
                polygonVertex[j++] = pols[i++];
              i++;
              p++;
            }
          } else {
            i += 2;
            while (i < n) {
              WORD vertices = pols[i++];
              obj->polygon[p].vertices = vertices;
              obj->polygon[p].index = j;
              while (--vertices >= 0)
                polygonVertex[j++] = pols[i++];
              p++;
            }
          }


          obj->polygonVertices = polygonVertices;
          obj->polygonVertex = polygonVertex;

          FreeMem(pols, polsLength);
        }
      }
    }

    CloseIff(&iff);
  }

  return obj;
}

void Load() {
  screen[0] = NewBitmap(WIDTH, HEIGHT, 1, FALSE);
  screen[1] = NewBitmap(WIDTH, HEIGHT, 1, FALSE);
  cube = LoadLWO("data/obj2.lwo", 140);
  cp = NewCopList(100);

  CopInit(cp);
  CopMakePlayfield(cp, bplptr, screen[0]);
  CopMakeDispWin(cp, X(0), Y(0), WIDTH, HEIGHT);
  CopSetRGB(cp, 0, 0x000);
  CopSetRGB(cp, 1, 0xfff);
  CopEnd(cp);
}

void Kill() {
  DeleteObject3D(cube);
  DeleteCopList(cp);
  DeleteBitmap(screen[0]);
  DeleteBitmap(screen[1]);
}

static void DrawObject(Object3D *object) {
  Point3D *point = object->cameraPoint;
  PolygonT *polygon = object->polygon;
  UWORD *vertex = object->polygonVertex;
  WORD polygons = object->polygons;
  Point3D *normal = object->polygonNormal;

  BlitterLineSetup(screen[active], 0, LINE_OR, LINE_SOLID);

  for (; polygons--; normal++, polygon++) {
    WORD i, n = polygon->vertices;
    Point3D out[16];

    for (i = 0; i < n; i++) {
      UWORD k = vertex[polygon->index + i];
      out[i] = point[k];
    }

    /* Perspective mapping. */
    for (i = 0; i < n; i++) {
      out[i].x = div16(256 * out[i].x, out[i].z) + 160;
      out[i].y = div16(256 * out[i].y, out[i].z) + 128;
    }

    for (i = 0; i < n; i++) {
      WORD j = i + 1;
      if (j >= n)
        j = 0;
      BlitterLine(out[i].x, out[i].y, out[j].x, out[j].y);
      WaitBlitter();
    }
  }
}

static Point3D rotate = { 0, 0, 0 };

static BOOL Loop() {
  Matrix3D t;

  BlitterClear(screen[active], 0);
  WaitBlitter();

  rotate.x += 4;
  rotate.y += 4;
  rotate.z += 4;

  {
    LONG lines = ReadLineCounter();

    LoadRotate3D(&t, rotate.x, rotate.y, rotate.z);
    Translate3D(&t, 0, 0, fx4i(-250));
    Transform3D(&t, cube->cameraPoint, cube->point, cube->points);
    DrawObject(cube);

    Log("object: %ld\n", ReadLineCounter() - lines);
  }

  WaitVBlank();
  CopInsSet32(bplptr[0], screen[active]->planes[0]);

  active ^= 1;

  return !LeftMouseButton();
}

void Main() {
  CopListActivate(cp);
  custom->dmacon = DMAF_SETCLR | DMAF_BLITTER | DMAF_RASTER;

  while (Loop());
}
