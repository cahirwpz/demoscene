#include "file.h"
#include "memory.h"
#include "reader.h"
#include "3d.h"

__regargs Object3D *LoadOBJ(char *filename) {
  char *file = ReadFile(filename, MEMF_PUBLIC);
  char *data = file;
  Object3D *object = NULL;
  WORD i, j, points, edges;

  if (!file)
    return NULL;
  
  if (ReadNumber(&data, &points) && ReadNumber(&data, &edges)) {
    object = NewObject3D(points, edges);

    for (i = 0; i < object->points; i++) {
      if (!ReadNumber(&data, &object->point[i].x) ||
          !ReadNumber(&data, &object->point[i].y) ||
          !ReadNumber(&data, &object->point[i].z))
        goto error;

      object->point[i].x *= 16;
      object->point[i].y *= 16;
      object->point[i].z *= 16;
    }

    /* Calculate size of polygonVertex array. */
    {
      char *ptr = data;

      for (i = 0; i < object->polygons; i++) {
        WORD n, tmp;

        if (!ReadNumber(&ptr, &n))
          goto error;

        object->polygonVertices += n + 1;

        while (n--) {
          if (!ReadNumber(&ptr, &tmp))
            goto error;
        }
      }
    }

    object->polygonVertex = MemAlloc(sizeof(UWORD) * object->polygonVertices,
                                     MEMF_PUBLIC);

    for (i = 0, j = 0; i < object->polygons; i++) {
      UWORD n, k;

      if (!ReadNumber(&data, &n))
        goto error;

      Log("Polygon %ld at %ld:", (LONG)i, (LONG)j);

      object->polygon[i].vertices = n + 1;
      object->polygon[i].index = j;

      k = j;

      while (n--) {
        UWORD tmp;

        if (!ReadNumber(&data, &tmp))
          goto error;

        object->polygonVertex[j++] = tmp;
        Log(" %ld", (LONG)tmp);
      }
      object->polygonVertex[j++] = object->polygonVertex[k];

      Log(" %ld\n", (LONG)object->polygonVertex[k]);
    }

    MemFreeAuto(file);
    return object;
  }

error:
  DeleteObject3D(object);
  MemFreeAuto(file);
  return NULL;
}
