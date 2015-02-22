#include "3d.h"
#include "ffp.h"
#include "iff.h"
#include "memory.h"

#define ID_LWOB MAKE_ID('L', 'W', 'O', 'B')
#define ID_LWO2 MAKE_ID('L', 'W', 'O', '2')
#define ID_PNTS MAKE_ID('P', 'N', 'T', 'S')
#define ID_POLS MAKE_ID('P', 'O', 'L', 'S')

__regargs Mesh3D *LoadLWO(char *filename, FLOAT scale) {
  Mesh3D *mesh = NULL;
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
        WORD vertices = pntsLength / 12;
        WORD faces = 0;
        WORD faceDataSize = 0;

        /* Count polygons and space they use. */
        {
          WORD *end = (APTR)pols + polsLength;

          if (iff.header.type == ID_LWOB) {
            WORD *data = pols;

            for (; data < end; faces++) {
              WORD count = *data++;
              faceDataSize += count + 1;
              data += count + 1;
            }
          } else {
            WORD *data = pols + 2;

            for (; data < end; faces++) {
              WORD count = *data++;
              faceDataSize += count + 1;
              data += count;
            }
          }
        }

        Log("File '%s' has %ld vertices and %ld faces.\n", 
            filename, (LONG)vertices, (LONG)faces);

        mesh = NewMesh3D(vertices, faces);
        mesh->faceData = MemAllocAuto(sizeof(WORD) * faceDataSize, MEMF_PUBLIC);

        /* Process vertices. */
        {
          FLOAT *src = pnts;
          WORD *dst = (WORD *)mesh->vertex;

          while (--vertices >= 0) {
            *dst++ = SPFix(SPMul(SPFieee(*src++), scale));
            *dst++ = SPFix(SPMul(SPFieee(*src++), scale));
            *dst++ = SPFix(SPMul(SPFieee(*src++), scale));
          }

          MemFree(pnts, pntsLength);
        }

        /* Process polygons. */
        {
          IndexListT **face = mesh->face;
          WORD *faceData = mesh->faceData;
          WORD *end = (APTR)pols + polsLength;
          WORD j = 0, p = 0;

          if (iff.header.type == ID_LWOB) {
            WORD *data = pols;

            while (data < end) {
              WORD count = *data++;
              face[p++] = (IndexListT *)&mesh->faceData[j];
              faceData[j++] = count;
              while (--count >= 0)
                faceData[j++] = *data++;
              data++;
            }
          } else {
            WORD *data = pols + 2;

            while (data < end) {
              WORD count = *data++;
              face[p++] = (IndexListT *)&mesh->faceData[j];
              faceData[j++] = count;
              while (--count >= 0)
                faceData[j++] = *data++;
            }
          }

          MemFree(pols, polsLength);
        }
      }
    }

    CloseIff(&iff);
  }

  return mesh;
}
