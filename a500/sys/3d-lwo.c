#include "3d.h"
#include "ffp.h"
#include "iff.h"
#include "memory.h"

#define ID_LWOB MAKE_ID('L', 'W', 'O', 'B')
#define ID_LWO2 MAKE_ID('L', 'W', 'O', '2')
#define ID_PNTS MAKE_ID('P', 'N', 'T', 'S')
#define ID_POLS MAKE_ID('P', 'O', 'L', 'S')
#define ID_PTAG MAKE_ID('P', 'T', 'A', 'G')
#define ID_SURF MAKE_ID('S', 'U', 'R', 'F')

static __regargs void ReadPNTS(Mesh3D *mesh, IffFileT *iff, FLOAT scale) {
  LONG length = iff->chunk.length;
  WORD vertices = length / 12;
  FLOAT *pnts = MemAlloc(length, MEMF_PUBLIC);

  mesh->vertices = vertices;
  mesh->vertex = MemAlloc(sizeof(Point3D) * vertices, MEMF_PUBLIC);

  ReadChunk(iff, pnts);

  Log(", %ld vertices", (LONG)vertices);

  /* Process vertices. */
  {
    FLOAT *src = pnts;
    WORD *dst = (WORD *)mesh->vertex;
    WORD i;

    scale = SPMul(scale, SPFlt(16));

    for (i = 0; i < vertices; i++) {
      WORD n = 3;
      while (--n >= 0) 
        *dst++ = SPFix(SPMul(SPFieee(*src++), scale));
      dst++;
    }
  }

  MemFree(pnts);
}

static __regargs void ReadPOLS(Mesh3D *mesh, IffFileT *iff) {
  LONG length = iff->chunk.length;
  WORD *pols = MemAlloc(length, MEMF_PUBLIC);
  WORD faces = 0;
  WORD faceDataSize = 0;

  ReadChunk(iff, pols);

  /* Count polygons and space they use. */
  {
    WORD *end = (APTR)pols + length;

    if (iff->header.type == ID_LWOB) {
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

  Log(", %ld faces", (LONG)faces);

  mesh->faces = faces;
  mesh->face = MemAlloc(sizeof(IndexListT *) * (faces + 1), MEMF_PUBLIC|MEMF_CLEAR);
  mesh->faceData = MemAlloc(sizeof(WORD) * faceDataSize, MEMF_PUBLIC);
  mesh->faceSurface = MemAlloc(faces, MEMF_PUBLIC|MEMF_CLEAR);

  /* Process polygons. */
  {
    IndexListT **face = mesh->face;
    BYTE *faceSurface = mesh->faceSurface;
    WORD *faceData = mesh->faceData;
    WORD *end = (APTR)pols + length;

    if (iff->header.type == ID_LWOB) {
      WORD *data = pols;

      while (data < end) {
        WORD count = *data++;
        *face++ = (IndexListT *)faceData;
        *faceData++ = count;
        while (--count >= 0)
          *faceData++ = *data++;
        *faceSurface++ = *data++;
      }
    } else {
      WORD *data = pols + 2;

      while (data < end) {
        WORD count = *data++;
        *face++ = (IndexListT *)faceData;
        *faceData++ = count;
        while (--count >= 0)
          *faceData++ = *data++;
      }
    }
  }

  MemFree(pols);
}

__regargs Mesh3D *LoadLWO(char *filename, FLOAT scale) {
  Mesh3D *mesh = NULL;
  IffFileT iff;

  if (OpenIff(&iff, filename)) {
    if (iff.header.type == ID_LWOB || iff.header.type == ID_LWO2) {
      mesh = MemAlloc(sizeof(Mesh3D), MEMF_PUBLIC|MEMF_CLEAR);

      Log("Reading '%s' file", filename);

      while (ParseChunk(&iff)) {
        switch (iff.chunk.type) {
          case ID_PNTS: ReadPNTS(mesh, &iff, scale); break;
          case ID_POLS: ReadPOLS(mesh, &iff); break;
          default: SkipChunk(&iff); break;
        }
      }

      Log(".\n");
    }

    CloseIff(&iff);
  } else {
    Log("File '%s' missing.\n", filename);
  }

  return mesh;
}
