#include "3d.h"
#include "ffp.h"
#include "iff.h"
#include "memory.h"

#define ID_LWOB MAKE_ID('L', 'W', 'O', 'B')
#define ID_LWO2 MAKE_ID('L', 'W', 'O', '2')
#define ID_PNTS MAKE_ID('P', 'N', 'T', 'S')
#define ID_POLS MAKE_ID('P', 'O', 'L', 'S')

__regargs Object3D *LoadLWO(char *filename, FLOAT scale) {
  Object3D *obj = NULL;
  IffFileT iff;

  scale = SPMul(scale, SPFlt(16));

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
            pnts = MemAlloc(pntsLength, MEMF_PUBLIC);
            ReadChunk(&iff, pnts);
            break;

          case ID_POLS:
            polsLength = iff.chunk.length;
            pols = MemAlloc(polsLength, MEMF_PUBLIC);
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
          for (i = 0; i < points; i++) {
            obj->point[i].x = SPFix(SPMul(SPFieee(pnts[i * 3 + 0]), scale));
            obj->point[i].y = SPFix(SPMul(SPFieee(pnts[i * 3 + 1]), scale));
            obj->point[i].z = SPFix(SPMul(SPFieee(pnts[i * 3 + 2]), scale));
          }

          MemFree(pnts, pntsLength);
        }

        /* Process polygons. */
        {
          WORD *polygonVertex = MemAlloc(sizeof(UWORD) * polygonVertices,
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

          MemFree(pols, polsLength);
        }
      }
    }

    CloseIff(&iff);
  }

  return obj;
}
