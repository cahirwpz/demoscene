#include <strings.h>
#include <memory.h>
#include <3d.h>
#include <fx.h>
#include <linkerset.h>

Object3D *NewObject3D(Mesh3D *mesh) {
  Object3D *object = MemAlloc(sizeof(Object3D), MEMF_PUBLIC|MEMF_CLEAR);
  short vertices = mesh->vertices;
  short faces = mesh->faces;
  short edges = mesh->edges;

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

void DeleteObject3D(Object3D *object) {
  if (object) {
    MemFree(object->visibleFace);
    MemFree(object->edgeFlags);
    MemFree(object->faceFlags);
    MemFree(object->vertexFlags);
    MemFree(object->vertex);
    MemFree(object);
  }
}

void UpdateObjectTransformation(Object3D *object) {
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
    short *camera = (short *)&object->camera;

    /* camera position in world space is (0, 0, 0) */
    short cx = M->x;
    short cy = M->y;
    short cz = M->z;

    *camera++ = normfx(M->m00 * cx + M->m01 * cy + M->m02 * cz);
    *camera++ = normfx(M->m10 * cx + M->m11 * cy + M->m12 * cz);
    *camera++ = normfx(M->m20 * cx + M->m21 * cy + M->m22 * cz);
  }
}

static char t_sqrt8[256];

void InitSqrtTab8(void){
  char *data = t_sqrt8;
  short i = 16;
  short n = 0;
  char k = 0;
  do {
    short j = n;
    do { *data++ = k; } while (--j != -1);
    k += 1;
    n += 2;
  } while (--i > 0);
}

ADD2INIT(InitSqrtTab8, 0);

void UpdateFaceVisibility(Object3D *object) {
  short *src = (short *)object->mesh->faceNormal;
  IndexListT **faces = object->mesh->face;
  char *faceFlags = object->faceFlags;
  void *vertex = object->mesh->vertex;
  short n = object->mesh->faces;
  char *sqrt = t_sqrt8;

  short *camera = (short *)&object->camera;

  while (--n >= 0) {
    IndexListT *face = *faces++;
    short px, py, pz;
    int f;

    {
      short *p = (short *)(vertex + (short)(*face->indices << 3));
      short *c = camera;
      px = *c++ - *p++;
      py = *c++ - *p++;
      pz = *c++ - *p++;
    }

    {
      int x = *src++ * px;
      int y = *src++ * py;
      int z = *src++ * pz;
      f = x + y + z;
    }

    src++;

    if (f >= 0) {
      /* normalize dot product */
      short l;
#if 0
      int s = px * px + py * py + pz * pz;
      s = swap16(s); /* s >>= 16, ignore upper word */
#else
      short s;
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
      l = div16((short)f * (short)f, s);
      if (l >= 256)
        *faceFlags++ = 15;
      else
        *faceFlags++ = sqrt[l];
    } else {
      *faceFlags++ = -1;
    }
  }
}

void UpdateVertexVisibility(Object3D *object) {
  char *vertexFlags = object->vertexFlags;
  char *faceFlags = object->faceFlags;
  IndexListT **faces = object->mesh->face;
  short n = object->mesh->faces;

  bzero(vertexFlags, object->mesh->vertices);

  while (--n >= 0) {
    IndexListT *face = *faces++;

    if (*faceFlags++ >= 0) {
      short *vi = face->indices;
      short count = face->count;

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

void SortFaces(Object3D *object) {
  IndexListT **faces = object->mesh->face;
  short n = object->mesh->faces;
  void *point = object->vertex;
  char *faceFlags = object->faceFlags;
  short count = 0;
  short index = 0;

  short *item = (short *)object->visibleFace;

  while (--n >= 0) {
    IndexListT *face = *faces++;

    if (*faceFlags++ >= 0) {
      short *vi = face->indices;
      short i1 = *vi++ << 3;
      short i2 = *vi++ << 3;
      short i3 = *vi++ << 3;
      short z = 0;

      z += *(short *)(point + i1 + 4);
      z += *(short *)(point + i2 + 4);
      z += *(short *)(point + i3 + 4);

      *item++ = z;
      *item++ = index;
      count++;
    }

    index++;
  }

  object->visibleFaces = count;

  SortItemArray(object->visibleFace, count);
}
