#include <strings.h>
#include "memory.h"
#include "io.h"
#include "reader.h"
#include "3d.h"
#include "fx.h"

__regargs Object3D *NewObject3D(Mesh3D *mesh) {
  Object3D *object = MemAlloc(sizeof(Object3D), MEMF_PUBLIC|MEMF_CLEAR);
  WORD vertices = mesh->vertices;
  WORD faces = mesh->faces;
  WORD edges = mesh->edges;

  object->mesh = mesh;
  object->vertex = MemAlloc(sizeof(Point3D) * vertices, MEMF_PUBLIC);
  object->vertexFlags = MemAlloc(vertices, MEMF_PUBLIC);
  object->faceFlags = MemAlloc(faces, MEMF_PUBLIC);
  if (edges)
    object->edgeFlags = MemAlloc(edges, MEMF_PUBLIC);
  object->visibleFace = MemAlloc(sizeof(SortItemT) * faces, MEMF_PUBLIC);

  object->scale.x = fx12f(1.0);
  object->scale.y = fx12f(1.0);
  object->scale.z = fx12f(1.0); 

  return object;
}

__regargs void DeleteObject3D(Object3D *object) {
  if (object) {
    MemFree(object->visibleFace);
    MemFree(object->edgeFlags);
    MemFree(object->faceFlags);
    MemFree(object->vertexFlags);
    MemFree(object->vertex);
    MemFree(object);
  }
}

__regargs void UpdateObjectTransformation(Object3D *object) {
  Point3D *rotate = &object->rotate;
  Point3D *scale = &object->scale;
  Point3D *translate = &object->translate;

  /* object -> world: Rx * Ry * Rz * S * T */
  {
    Matrix3D *m = &object->objectToWorld;
    LoadRotate3D(m, rotate->x, rotate->y, rotate->z);
    Scale3D(m, scale->x, scale->y, scale->z);
    Translate3D(m, translate->x, translate->y, translate->z);
  }

  /* world -> object: T * S * Rz * Ry * Rx */
  {
    Matrix3D *m = &object->worldToObject;
    Matrix3D m_scale, m_rotate;

    LoadIdentity3D(&m_scale);

    /*
     * Translation is formally first, and we achieve that in ReverseTransform3D.
     * Matrix3D is used only to store the vector.
     */
    m_scale.x = -translate->x;
    m_scale.y = -translate->y;
    m_scale.z = -translate->z;

    m_scale.m00 = div16(1 << 24, scale->x);
    m_scale.m11 = div16(1 << 24, scale->y);
    m_scale.m22 = div16(1 << 24, scale->z);

    LoadReverseRotate3D(&m_rotate, -rotate->x, -rotate->y, -rotate->z);
    Compose3D(m, &m_scale, &m_rotate);
  }

  /* calculate camera position in object space */ 
  {
    Matrix3D *M = &object->worldToObject;
    WORD *camera = (WORD *)&object->camera;

    /* camera position in world space is (0, 0, 0) */
    WORD cx = M->x;
    WORD cy = M->y;
    WORD cz = M->z;

    *camera++ = normfx(M->m00 * cx + M->m01 * cy + M->m02 * cz);
    *camera++ = normfx(M->m10 * cx + M->m11 * cy + M->m12 * cz);
    *camera++ = normfx(M->m20 * cx + M->m21 * cy + M->m22 * cz);
  }
}

static BYTE t_sqrt8[256];

void InitSqrtTab8(void){
  BYTE *data = t_sqrt8;
  WORD i = 16;
  WORD n = 0;
  BYTE k = 0;
  do {
    WORD j = n;
    do { *data++ = k; } while (--j != -1);
    k += 1;
    n += 2;
  } while (--i > 0);
}

ADD2INIT(InitSqrtTab8, 0);

__regargs void UpdateFaceVisibility(Object3D *object) {
  WORD *src = (WORD *)object->mesh->faceNormal;
  IndexListT **faces = object->mesh->face;
  BYTE *faceFlags = object->faceFlags;
  APTR vertex = object->mesh->vertex;
  WORD n = object->mesh->faces;
  BYTE *sqrt = t_sqrt8;

  WORD *camera = (WORD *)&object->camera;

  while (--n >= 0) {
    IndexListT *face = *faces++;
    WORD px, py, pz;
    LONG f;

    {
      WORD *p = (WORD *)(vertex + (WORD)(*face->indices << 3));
      WORD *c = camera;
      px = *c++ - *p++;
      py = *c++ - *p++;
      pz = *c++ - *p++;
    }

    {
      LONG x = *src++ * px;
      LONG y = *src++ * py;
      LONG z = *src++ * pz;
      f = x + y + z;
    }

    src++;

    if (f >= 0) {
      /* normalize dot product */
      WORD l;
#if 0
      LONG s = px * px + py * py + pz * pz;
      s = swap16(s); /* s >>= 16, ignore upper word */
#else
      WORD s;
      asm("mulsw %0,%0\n"
          "mulsw %1,%1\n"
          "mulsw %2,%2\n"
          "addl  %1,%0\n"
          "addl  %2,%0\n"
          "swap  %0\n"
          : "+d" (px), "+d" (py), "+d" (pz));
      s = px;
#endif
      f = swap16(f); /* f >>= 16, ignore upper word */
      l = div16((WORD)f * (WORD)f, s);
      if (l >= 256)
        *faceFlags++ = 15;
      else
        *faceFlags++ = sqrt[l];
    } else {
      *faceFlags++ = -1;
    }
  }
}

__regargs void UpdateVertexVisibility(Object3D *object) {
  BYTE *vertexFlags = object->vertexFlags;
  BYTE *faceFlags = object->faceFlags;
  IndexListT **faces = object->mesh->face;
  WORD n = object->mesh->faces;

  bzero(vertexFlags, object->mesh->vertices);

  while (--n >= 0) {
    IndexListT *face = *faces++;

    if (*faceFlags++ >= 0) {
      WORD *vi = face->indices;
      WORD count = face->count;

      /* Face has at least (and usually) three vertices. */
      switch (count) {
        case 6: vertexFlags[*vi++] = -1;
        case 5: vertexFlags[*vi++] = -1;
        case 4: vertexFlags[*vi++] = -1;
        case 3: vertexFlags[*vi++] = -1;
                vertexFlags[*vi++] = -1;
                vertexFlags[*vi++] = -1;
        default: break;
      }
    }
  }
}

__regargs void SortFaces(Object3D *object) {
  IndexListT **faces = object->mesh->face;
  WORD n = object->mesh->faces;
  APTR point = object->vertex;
  BYTE *faceFlags = object->faceFlags;
  WORD count = 0;
  WORD index = 0;

  WORD *item = (WORD *)object->visibleFace;

  while (--n >= 0) {
    IndexListT *face = *faces++;

    if (*faceFlags++ >= 0) {
      WORD *vi = face->indices;
      WORD i1 = *vi++ << 3;
      WORD i2 = *vi++ << 3;
      WORD i3 = *vi++ << 3;
      WORD z = 0;

      z += *(WORD *)(point + i1 + 4);
      z += *(WORD *)(point + i2 + 4);
      z += *(WORD *)(point + i3 + 4);

      *item++ = z;
      *item++ = index;
      count++;
    }

    index++;
  }

  object->visibleFaces = count;

  SortItemArray(object->visibleFace, count);
}
